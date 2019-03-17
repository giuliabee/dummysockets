#include "sw.h"
#define  WIN               // WIN for Winsock and BSD for BSD sockets

#include <stdio.h>          // Needed for printf()
#include <string.h>         // Needed for memcpy() and strcpy()
#include <stdlib.h>         // Needed for exit()
#include <fcntl.h>          // Needed for file i/o constants
#include <string.h>
#include <ctype.h>
#include <signal.h>
#ifdef _WIN
  #include <windows.h>      // Needed for all Winsock stuff
  #include <io.h>             // Needed for open(), close(), and eof()
  #include <sys\stat.h>       // Needed for file i/o constants
  #include <ws2tcpip.h>
  #include <winsock2.h>
  typedef int socklen_t;
#else
  #include <sys/types.h>
  #include <netinet/in.h>
  #include <sys/socket.h>
  #include <arpa/inet.h>
  #include <fcntl.h>
  #include <netdb.h>
  #include <sys/io.h>       // Needed for open(), close(), and eof()
  #include <sys/stat.h>     // Needed for file i/o constants
  #include <unistd.h>
  #include <sys/select.h>
  #define closesocket(s) close(s)
  typedef int SOCKET;
  typedef struct sockaddr_in SOCKADDR_IN;
  typedef struct sockaddr SOCKADDR;
#endif

void ListCommandLines(){
    printf("exit\n");
    printf("help\n");
    printf("listu\n");
    printf("listf\n");
    printf("trfD\n");
    printf("trfU\n");
    printf("private\n");
    printf("public\n");
    printf("ring\n");
    printf("original\n");
    printf("logs\n");
}

void cleanExit() { exit(0); }

#define  PORT_NUM    6009    // Port number used at the server for send and receive file
#define  TIMEOUT 2

#define EXIT_SUCCESS    0
#define EXIT_ERROR      1

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_SIZE 280

#define EXIT_NOTIFICATION   "close\n"

typedef struct //for send and receive a file
{
    int socket;
    struct sockaddr *addr;
    socklen_t addr_len;
} transport_info_t;

//----- Prototypes ------------------------------------------------------------
int sendFile(char *fileName, char *destIpAddr, int destPortNum, int timeout); //for send and receive a file

int main() {
  //Chat();
  TransferFile();
}

int TransferFile(int argc, char *argv[]){
  char                 sendFileName[256];   // Send file name
  char                 recv_ipAddr[16];     // Reciver IP address
  int                  recv_port;           // Receiver port number
  int                  timeout;             // Options
  int                  retcode;             // Return code

  // Usage and parsing command line arguments
  if (argc != 4)
  {
   printf("to launch the program, first write as the following: 'nameOfProject.exe fileNameToBeSent.txt IpAddress 6009'\n");
    return(0);
  }
  strcpy(sendFileName, argv[1]);
  strcpy(recv_ipAddr, argv[2]);
  recv_port = atoi(argv[3]);

  // Initialize parameters
  timeout = TIMEOUT;

  // Send the file
  printf("Starting file transfer... \n");
  retcode = sendFile(sendFileName, recv_ipAddr, recv_port, timeout);
  if (retcode)
      return -1;

  printf("File transfer is complete \n");

  // Return
  return(0);
}

int Chat(){
  int sock, err, i, bytes, len;
  char msg[BUFFER_SIZE];

  struct sockaddr_in dest_addr;

  signal(SIGTERM, cleanExit);
  signal(SIGINT, cleanExit);

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
      perror("Unable to create socket");
      return EXIT_ERROR;
  }

  /* initialize address */
  memset((void *) &dest_addr, 0, sizeof(dest_addr));    /* clear server address */
  dest_addr.sin_family = AF_INET;                       /* address type is INET */
  dest_addr.sin_port = htons(SERVER_PORT);

  /* Convert the address from strin gform to binary form */
  err = inet_pton(AF_INET, SERVER_ADDRESS, &dest_addr.sin_addr);
  if (err <= 0) {
      perror("Address creation error");
      return EXIT_ERROR;
  }

  while (-1) {
      printf("Enter message: \n");

      fgets(msg, BUFFER_SIZE, stdin);

      len = strlen(msg);

      sendto(sock, &msg, len, 0, (struct sockaddr *) &dest_addr, sizeof(struct sockaddr_in));

      printf("Sent %d bytes\n", len);

      bytes = recv(sock, msg, BUFFER_SIZE, 0);

      if (bytes < 0) {
          perror("Failed to receive");
          return EXIT_ERROR;
      }

      printf("Received %d bytes: %s", bytes, msg);

      err = strcmp(msg, EXIT_NOTIFICATION);

      if (err == 0) {
          printf("Server closed udp socket\n");
          return EXIT_SUCCESS;
      }
  }
}

#if defined (WIN32)
//from https://stackoverflow.com/questions/15660203/inet-pton-identifier-not-found
int inet_pton(int af, const char *src, void *dst) {
    struct sockaddr_storage ss;
    int size = sizeof(ss);
    char src_copy[INET6_ADDRSTRLEN + 1];

    ZeroMemory(&ss, sizeof(ss));
    /* stupid non-const API */
    strncpy(src_copy, src, INET6_ADDRSTRLEN + 1);
    src_copy[INET6_ADDRSTRLEN] = 0;

    if (WSAStringToAddress(src_copy, af, NULL, (struct sockaddr *) &ss, &size) == 0) {
        switch (af) {
            case AF_INET:
                *(struct in_addr *) dst = ((struct sockaddr_in *) &ss)->sin_addr;
                return 1;
            case AF_INET6:
                *(struct in6_addr *) dst = ((struct sockaddr_in6 *) &ss)->sin6_addr;
                return 1;
        }
    }
    return 0;
}


const char *inet_ntop(int af, const void *src, char *dst, socklen_t size) {
    struct sockaddr_storage ss;
    unsigned long s = size;

    ZeroMemory(&ss, sizeof(ss));
    ss.ss_family = af;

    switch (af) {
        case AF_INET:
            ((struct sockaddr_in *) &ss)->sin_addr = *(struct in_addr *) src;
            break;
        case AF_INET6:
            ((struct sockaddr_in6 *) &ss)->sin6_addr = *(struct in6_addr *) src;
            break;
        default:
            return NULL;
    }
    /* cannot directly use &size because of strict aliasing rules */
    return (WSAAddressToString((struct sockaddr *) &ss, sizeof(ss), NULL, dst, &s) == 0) ?
           dst : NULL;
}
#endif

static int wait_for_data_timeout(int timeout, void *ctx)
{
    int socket = *(int *)ctx;
    fd_set fds;
    struct timeval tv;

    FD_ZERO(&fds);
    FD_SET(socket, &fds);

    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    return select(socket + 1, &fds, NULL, NULL, &tv);
}

static int send_data(void *data, int data_len, void *ctx)
{
    int ret_code;
    transport_info_t *transport = (transport_info_t *)ctx;

    ret_code = sendto(transport->socket, data, data_len, 0, transport->addr,
	transport->addr_len);

    return ret_code;
}

static int read_data_from_file(void *buffer, int buffer_size, void *ctx)
{
    int length, fd = *(int *)ctx;

    length = read(fd, buffer, buffer_size);

    return length;
}

static int receive_data(void *buffer, int buffer_size, void *ctx)
{
    int length;
    transport_info_t *transport = (transport_info_t *)ctx;

    length = recvfrom(transport->socket, buffer, buffer_size, 0, NULL, NULL);

    return length;
}


int sendFile(char *fileName, char *destIpAddr, int destPortNum, int timeout)
{
#ifdef _WIN
  WORD wVersionRequested = MAKEWORD(1,1);       // Stuff for WSA functions
  WSADATA wsaData;                              // Stuff for WSA functions
#endif
  int                  client_s;        // Client socket descriptor
  struct sockaddr_in   server_addr;     // Server Internet address
  int                  fh;              // File handle
  int                  retcode;         // Return code
  transport_info_t transport = {};

#ifdef _WIN
  // This stuff initializes winsock
  WSAStartup(wVersionRequested, &wsaData);
#endif

  // Create a client socket
  client_s = socket(AF_INET, SOCK_DGRAM, 0);
  if (client_s < 0)
  {
    printf("*** ERROR - socket() failed \n");
    exit(-1);
  }

  // Fill-in the server's address information and do a connect
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(destPortNum);
  server_addr.sin_addr.s_addr = inet_addr(destIpAddr);

  transport.addr = (struct sockaddr *)&server_addr;
  transport.addr_len = sizeof(server_addr);
  transport.socket = client_s;

  // Open file to send
  #ifdef _WIN
    fh = open(fileName, O_RDONLY | O_BINARY, S_IREAD | S_IWRITE);
  #else
    fh = open(fileName, O_RDONLY, S_IREAD | S_IWRITE);
  #endif
  if (fh == -1)
  {
     printf("  *** ERROR - unable to open '%p' \n", sendFile);
     exit(1);
  }
  //use several functions froms header of sw
  sw_ctx_t *sw_ctx = sw_init(timeout);
  sw_set_cb(sw_ctx, SW_SEND_CB, send_data, &transport);
  sw_set_cb(sw_ctx, SW_RECV_CB, receive_data, &transport);
  sw_set_cb(sw_ctx, SW_READ_DATA_CB, read_data_from_file, &fh);
  sw_set_cb(sw_ctx, SW_TIMEOUT_CB, wait_for_data_timeout, &client_s);

  retcode = sw_send_file(sw_ctx);
  if (retcode)
  {
      printf("*** ERROR - file send failed \n");
      exit(-1);
  }

  sw_deinit(sw_ctx);
  // Close the file that was sent to the receiver
  close(fh);

  // Close the client socket
#ifdef _WIN
  retcode = closesocket(client_s);
  if (retcode < 0)
  {
    printf("*** ERROR - closesocket() failed \n");
    exit(-1);
  }
#else
  retcode = close(client_s);
  if (retcode < 0)
  {
    printf("*** ERROR - close() failed \n");
    exit(-1);
  }
#endif

#ifdef _WIN
  // Clean-up winsock
  WSACleanup();
#endif

  // Return zero
  return(0);
}
