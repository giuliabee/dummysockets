#define  WIN               // WIN for Winsock and BSD for BSD sockets

#include <stdio.h>          // Needed for printf()
#include <string.h>         // Needed for memcpy() and strcpy()
#include <stdlib.h>         // Needed for exit()
#include <fcntl.h>          // Needed for file i/o constants
#include <string.h>
#include <ctype.h>
#ifdef WIN
  #include <windows.h>      // Needed for all Winsock stuff
  #include <io.h>             // Needed for open(), close(), and eof()
  #include <sys\stat.h>       // Needed for file i/o constants
  #include <ws2tcpip.h>
#endif
#ifdef BSD
  #include <sys/types.h>    // Needed for sockets stuff
  #include <netinet/in.h>   // Needed for sockets stuff
  #include <sys/socket.h>   // Needed for sockets stuff
  #include <arpa/inet.h>    // Needed for sockets stuff
  #include <fcntl.h>        // Needed for sockets stuff
  #include <netdb.h>        // Needed for sockets stuff
  #include <sys/io.h>       // Needed for open(), close(), and eof()
  #include <sys/stat.h>     // Needed for file i/o constants
  #include <unistd.h>
  #include <sys/select.h>
#endif

#include "sw.h"
//----- Defines ---------------------------------------------------------------
#define  PORT_NUM    6009    // Port number used at the server
#define  TIMEOUT 2

typedef struct
{
    int socket;
    struct sockaddr *addr;
    socklen_t addr_len;
} transport_info_t;

//----- Prototypes ------------------------------------------------------------
int sendFile(char *fileName, char *destIpAddr, int destPortNum, int timeout);

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

//===== Main program ==========================================================
int main(int argc, char *argv[])
{
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

int sendFile(char *fileName, char *destIpAddr, int destPortNum, int timeout)
{
#ifdef WIN
  WORD wVersionRequested = MAKEWORD(1,1);       // Stuff for WSA functions
  WSADATA wsaData;                              // Stuff for WSA functions
#endif
  int                  client_s;        // Client socket descriptor
  struct sockaddr_in   server_addr;     // Server Internet address
  int                  fh;              // File handle
  int                  retcode;         // Return code
  transport_info_t transport = {};

#ifdef WIN
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
  #ifdef WIN
    fh = open(fileName, O_RDONLY | O_BINARY, S_IREAD | S_IWRITE);
  #endif
  #ifdef BSD
    fh = open(fileName, O_RDONLY, S_IREAD | S_IWRITE);
  #endif
  if (fh == -1)
  {
     printf("  *** ERROR - unable to open '%p' \n", sendFile);
     exit(1);
  }

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
#ifdef WIN
  retcode = closesocket(client_s);
  if (retcode < 0)
  {
    printf("*** ERROR - closesocket() failed \n");
    exit(-1);
  }
#endif
#ifdef BSD
  retcode = close(client_s);
  if (retcode < 0)
  {
    printf("*** ERROR - close() failed \n");
    exit(-1);
  }
#endif

#ifdef WIN
  // Clean-up winsock
  WSACleanup();
#endif

  // Return zero
  return(0);
}