#define  WIN               // WIN for Winsock and BSD for BSD sockets
//----- Include files ---------------------------------------------------------
#include <stdio.h>          // Needed for printf()
#include <string.h>         // Needed for memcpy() and strcpy()
#include <stdlib.h>         // Needed for exit()
#include <fcntl.h>          // Needed for file i/o constants
#include <ctype.h>
#ifdef _WIN
  #include <windows.h>      // Needed for all Winsock stuff
  #include <io.h>           // Needed for open(), close(), and eof()
  #include <sys\stat.h>     // Needed for file i/o constants
#else
  #include <sys/types.h>    // Needed for sockets stuff
  #include <netinet/in.h>   // Needed for sockets stuff
  #include <sys/socket.h>   // Needed for sockets stuff
  #include <arpa/inet.h>    // Needed for sockets stuff
  #include <fcntl.h>        // Needed for sockets stuff
  #include <netdb.h>        // Needed for sockets stuff
  #include <sys/io.h>       // Needed for open(), close(), and eof()
  #include <sys/stat.h>     // Needed for file i/o constants
  #include <pthread.h>
  #include <unistd.h>
#endif

#define EXIT_SUCCESS    0
#define EXIT_ERROR      1

#define  PORT_NUM    6069           // Arbitrary port number for the server
#define  SIZE        256            // Buffer size
#define  RECV_FILE  "recvFile.dat"  // File name of received file

#define SERVER_PORT 12345
#define BUFFER_SIZE 280

#define NUM_THREADS 50

#define EXIT_NOTIFICATION   "close\n"

typedef struct Request_s {
    int sock;
    socklen_t fromlen;
    struct sockaddr_in from;
} Request;

void *serve_udp_client(void *req) {
    int sock, err;
    size_t len;
    ssize_t bytes;

    char msg[BUFFER_SIZE];

    struct sockaddr_in client_addr;

    Request *request = (Request *) req;

    sock = request->sock;

    while (1) {
        printf("Enter message for udp client %d (type 'close' to end communication):\n", sock);
        fgets(msg, BUFFER_SIZE, stdin);

        len = strlen(msg);

        bytes = sendto(sock, &msg, len, 0, (struct sockaddr *) &request->from, request->fromlen);
        printf("Sent %ld bytes to udp client %d\n", bytes, sock);

        if (bytes <= 0) {
            /* Error or connection closed */
            break;
        }

        err = strcmp(msg, EXIT_NOTIFICATION);

        if (err == 0) {
            printf("Ending udp communication with client %d\n", sock);
            return EXIT_SUCCESS;
        }

        bytes = recvfrom(sock, &msg, BUFFER_SIZE, 0, (struct sockaddr *) &request->from, &request->fromlen);

        if (bytes <= 0) {
            /* Error or connection closed */
            break;
        }

        printf("Received %ld bytes from udp client %d\n", bytes, sock);
        printf("%s", msg);
    }
}

void *serve_tcp_client(void *client_sock) {
    int sock, bytes, len, err;

    char msg[BUFFER_SIZE];

    struct sockaddr_in client_addr;

    sock = (long) client_sock;

    while (1) {
        bytes = recv(sock, &msg, BUFFER_SIZE, 0);

        if (bytes <= 0) {
            /* Error or connection closed */
            break;
        }

        printf("Received %d bytes from tcp client %d\n", bytes, sock);

        printf("%s", msg);

        printf("Enter message for tcp client %d (type 'close' to end communication):\n", sock);

        fgets(msg, BUFFER_SIZE, stdin);

        len = strlen(msg);

        bytes = send(sock, &msg, len, 0);

        printf("Sent %d bytes to tcp client %d\n", bytes, sock);

        if (bytes <= 0) {
            /* Error or connection closed */
            break;
        }

        err = strcmp(msg, EXIT_NOTIFICATION);

        if (err == 0) {
            printf("Closing tcp client socket %d\n", sock);
            shutdown(sock, 2);
            return EXIT_SUCCESS;
        }
    }
}

int main() {
//Chat();
Receivingfile();
}

int Chat(){
  int listen_sock, client_sock, client_addr_size, bytes, err, i;

  struct sockaddr_in server_addr, client_addr;

  pid_t child;

  child = fork();


  if (child == 0) {
      /***** udp *****/
      pthread_t threads[NUM_THREADS];
      char msg[BUFFER_SIZE];


      client_sock = socket(AF_INET, SOCK_DGRAM, 0);
      if (client_sock < 0) {
          perror("Unable to create udp socket\n");
          return EXIT_ERROR;
      }

      /* initialize address */
      memset((void *) &server_addr, 0, sizeof(server_addr));    /* clear server address */
      server_addr.sin_family = AF_INET;                       /* address type is INET */
      server_addr.sin_port = htons(SERVER_PORT);
      server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

      /* Bind the port number to my process */
      err = bind(client_sock, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in));
      if (err < 0) {
          printf("Unable to bind udp socket\n");
          return EXIT_ERROR;
      }

      for (i = 0; i < NUM_THREADS; i++) {

          struct Request_s *request = (struct Request_s *) malloc(sizeof(Request));
          request->sock = client_sock;
          request->fromlen = sizeof(struct sockaddr_in);

          printf("Serving new udp client\n");

          bytes = recvfrom(client_sock, &msg, BUFFER_SIZE, 0, (struct sockaddr *) &request->from, &request->fromlen);

          if (bytes <= 0) {
              /* Error or connection closed */
              break;
          }

          printf("Received %d bytes from udp client %d\n", bytes, client_sock);
          printf("%s", msg);

         pthread_create(&threads[i], NULL, serve_udp_client, (void *) request);
      }

  } else {

      /***** tcp *****/

      pthread_t thread;

      listen_sock = socket(AF_INET, SOCK_STREAM, 0);
      if (listen_sock < 0) {
          perror("Unable to create tcp socket\n");
          return EXIT_ERROR;

      }

      /* initialize address */
      memset((void *) &server_addr, 0, sizeof(server_addr));    /* clear server address */
      server_addr.sin_family = AF_INET;                       /* address type is INET */
      server_addr.sin_port = htons(SERVER_PORT);
      server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

      /* Bind the port number to my process */
      err = bind(listen_sock, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in));
      if (err < 0) {
          perror("Unable to bind tcp socket\n");
          return EXIT_ERROR;
      }

      err = listen(listen_sock, 10);
      if (err < 0) {
          perror("Unable to listen tcp socket\n");
          return EXIT_ERROR;
      }
      /* We are now connected */

      while (1) {
          client_addr_size = sizeof(struct sockaddr_in);

          client_sock = accept(listen_sock, (struct sockaddr *) &client_addr, &client_addr_size);

          if (client_sock < 0) {
              perror("accept() failed\n");
              continue;
          }

          printf("Serving new tcp client\n");

          pthread_create(&thread, NULL, serve_tcp_client, (void *) client_sock);
      }

  }
}

int recvFile(char *fileName, int portNum, int maxSize, int options);

int Receivingfile(){
  int                  portNum;         // Port number to receive on
  int                  maxSize;         // Maximum allowed size of file
  int                  timeOut;         // Timeout in seconds
  int                  options;         // Options
  int                  retcode;         // Return code

  // Initialize parameters
  portNum = PORT_NUM;
  maxSize = 0;     // This parameter is unused in this implementation
  options = 0;     // This parameter is unused in this implementation

  // Receive the file
  printf("Starting file reception... \n");
  retcode = recvFile(RECV_FILE, portNum, maxSize, options);
  printf("File received \n");

  // Return
  return(0);
}

int recvFile(char *fileName, int portNum, int maxSize, int options)
{
#ifdef _WIN
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

#ifdef _WIN
  // This stuff initializes winsock
  WSAStartup(wVersionRequested, &wsaData);
#endif
//printf("Coucou dans la boucle de la fonction received");
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
  #ifdef _WIN
    fh = open(fileName, O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IREAD | S_IWRITE);
  #else
    fh = open(fileName, O_WRONLY | O_CREAT | O_TRUNC, S_IREAD | S_IWRITE);
  #endif
  //printf(fh);
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
#ifdef _WIN
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
#else
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

#ifdef _WIN
  // Clean-up winsock
  WSACleanup();
#endif

  // Return zero
  return(0);
}
