#if defined (WIN32)
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #include <fcntl.h>
    #include <ctype.h>
    typedef int socklen_t;
#elif defined (linux)
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <arpa/inet.h>
    #include <fcntl.h>
    #include <netdb.h>
    #include <sys/io.h>
    #include <sys/stat.h>
    #include <netinet/in.h>
    #define closesocket(s) close(s)
    typedef int SOCKET;
    typedef struct sockaddr_in SOCKADDR_IN;
    typedef struct sockaddr SOCKADDR;
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>


#define  PORT_NUM    6069
#define  SIZE        256
#define  RECV_FILE  "receivedFile.dat"

int recvFile(char *fileName, int portNum, int maxSize, int options);
void cleanExit() { exit(0); }

#define EXIT_SUCCESS    0
#define EXIT_ERROR      1

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_SIZE 280

#define EXIT_NOTIFICATION   "close\n"

int main() {
  char userResponse;
  printf("What do you want to start :\n");
  printf("1- Chat\n");
  printf("2- File\n");


    scanf(" %c\n", userResponse );
    if(userResponse == 1){
      chat();
      break;
    }
    if(userResponse == 2){
      //file();
      break;
    }
    system("clear");


}

int file(){
  int                  portNum;         // Port number to receive on
  int                  maxSize;         // Maximum allowed size of file
  int                  timeOut;         // Timeout in seconds
  int                  options;         // Options
  int                  retcode;         // Return code

  // Initialize parameters
  portNum = PORT_NUM;

  // Receive the file
  printf("Starting file reception... \n");
  retcode = recvFile(RECV_FILE, portNum, maxSize, options);
  printf("File received \n");

  // Return
  return(0);
}


int recvFile(char *fileName, int portNum, int maxSize, int options)
{
#ifdef WIN
  WORD wVersionRequested = MAKEWORD(1,1);       // Stuff for WSA functions
  WSADATA wsaData;                              // Stuff for WSA functions
#endif
  int                  welcome_s;       // Welcome socket descriptor
  struct sockaddr_in   server_addr;     // Server Internet address
  int                  connect_s;       // Connection socket descriptor
  struct sockaddr_in   client_addr;     // Client Internet address
  struct in_addr       client_ip_addr;  // Client IP address
  int                  addr_len;        // Internet address length
  char                 in_buf[4096];    // Input buffer for data
  int                  fh;              // File handle
  int                  length;          // Length in received buffer
  int                  retcode;         // Return code

#ifdef WIN
  // This stuff initializes winsock
  WSAStartup(wVersionRequested, &wsaData);
#endif

  // Create a welcome socket
  welcome_s = socket(AF_INET, SOCK_STREAM, 0);
  if (welcome_s < 0)
  {
    printf("*** ERROR - socket() failed \n");
    exit(-1);
  }

  // Fill-in server (my) address information and bind the welcome socket
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(portNum);
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  retcode = bind(welcome_s, (struct sockaddr *)&server_addr,
    sizeof(server_addr));
  if (retcode < 0)
  {
    printf("*** ERROR - bind() failed \n");
    exit(-1);
  }

  // Listen on welcome socket for a connection
  listen(welcome_s, 1);

  // Accept a connection
  addr_len = sizeof(client_addr);
  connect_s = accept(welcome_s, (struct sockaddr *)&client_addr, &addr_len);
  if (connect_s < 0)
  {
    printf("*** ERROR - accept() failed \n");
    exit(-1);
  }

  // Open IN_FILE for file to write
  #ifdef WIN
    fh = open(fileName, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE);
  #endif
  #ifdef BSD
    fh = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
  #endif
  if (fh == -1)
  {
     printf("  *** ERROR - unable to create '%s' \n", RECV_FILE);
     exit(1);
  }

  // Receive and write file from tcpFileSend
  do
  {
    length = recv(connect_s, in_buf, SIZE, 0);
    if (length < 0)
    {
      printf("*** ERROR - recv() failed \n");
      exit(-1);
    }
    if (length > 0)
      write(fh, in_buf, length);
  } while (length > 0);

  // Close the received file
  close(fh);

  // Close the welcome and connect sockets
#ifdef WIN
  retcode = closesocket(welcome_s);
  if (retcode < 0)
  {
    printf("*** ERROR - closesocket() failed \n");
    exit(-1);
  }
  retcode = closesocket(connect_s);
  if (retcode < 0)
  {
    printf("*** ERROR - closesocket() failed \n");
    exit(-1);
  }
#endif
#ifdef BSD
  retcode = close(welcome_s);
  if (retcode < 0)
  {
    printf("*** ERROR - close() failed \n");
    exit(-1);
  }
  retcode = close(connect_s);
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



int chat(){
  int sock, err, bytes, len;
  char msg[BUFFER_SIZE];

  struct sockaddr_in dest_addr;

  signal(SIGTERM, cleanExit);
  signal(SIGINT, cleanExit);

  sock = socket(AF_INET, SOCK_STREAM, 0);
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

  err = connect(sock, (struct sockaddr *) &dest_addr, sizeof(dest_addr));
  if (err < 0) {
      perror("Error connecting to the server");
      return EXIT_ERROR;
  }

  /* We are now connected */
  while (-1) {
      printf("Enter message: \n");

      fgets(msg, BUFFER_SIZE, stdin);

      len = strlen(msg);

      bytes = send(sock, msg, len, 0);

      if (bytes < 0) {
          perror("Failed to send");
          return EXIT_ERROR;
      }

      printf("Sent %d bytes\n", bytes);

      bytes = recv(sock, msg, BUFFER_SIZE, 0);

      if (bytes < 0) {
          perror("Failed to receive");
          return EXIT_ERROR;
      }

      printf("Received %d bytes: %s", bytes, msg);

      err = strcmp(msg, EXIT_NOTIFICATION);

      if (err == 0) {
          printf("Server closed tcp socket\n");
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
