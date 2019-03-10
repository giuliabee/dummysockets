#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

#define  PORT_NUM    6069
#define  SIZE        256

int sendFile(char *fileName, char *destIpAddr, int destPortNum, int options);
void cleanExit() { exit(0); }

#define EXIT_SUCCESS    0
#define EXIT_ERROR      1

#define SERVER_PORT 12345
#define BUFFER_SIZE 280

#define EXIT_NOTIFICATION   "close\n"

#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;


int main() {
  char userResponse;
  printf("What do you want to start :\n");
  printf("1- Chat\n");
  printf("2- File\n");


    scanf(" %c\n", userResponse );
    if(userResponse == 1){
      listen();
      break;
    }
    if(userResponse == 2){
      start_to_send_file();
      break;
    }
    system("clear");
}

int start_to_send_file(){
  char                 sendFileName[256];   // Send file name
  char                 recv_ipAddr[16];     // Receiver IP address
  int                  recv_port;           // Receiver port number
  int                  retcode;

  // Usage and parsing command line arguments
  if (argc != 4)
  {
    printf("to launch the program, first write as the following: 'nameOfProject.exe fileNameToBeSent.txt IpAddress 6069'\n");
    return(0);
  }
  strcpy(sendFileName, argv[1]);
  strcpy(recv_ipAddr, argv[2]);
  recv_port = atoi(argv[3]);

  // Initialize parameters

  // Send the file
  printf("Starting file transfer... \n");
  retcode = sendFile(sendFileName, recv_ipAddr, recv_port, options);
  printf("File transfer is complete \n");

  // Return
  return(0);
}

int sendFile(char *fileName, char *destIpAddr, int destPortNum, int options)
{
#ifdef WIN
  WORD wVersionRequested = MAKEWORD(1,1);       // Stuff for WSA functions
  WSADATA wsaData;                              // Stuff for WSA functions
#endif
  int                  client_s;        // Client socket descriptor
  struct sockaddr_in   server_addr;     // Server Internet address
  char                 out_buf[4096];   // Output buffer for data
  int                  fh;              // File handle
  int                  length;          // Length of send buffer
  int                  retcode;         // Return code

#ifdef WIN
  // This stuff initializes winsock
  WSAStartup(wVersionRequested, &wsaData);
#endif

  // Create a client socket
  client_s = socket(AF_INET, SOCK_STREAM, 0);
  if (client_s < 0)
  {
    printf("*** ERROR - socket() failed \n");
    exit(-1);
  }

  // Fill-in the server's address information and do a connect
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(destPortNum);
  server_addr.sin_addr.s_addr = inet_addr(destIpAddr);
  retcode = connect(client_s, (struct sockaddr *)&server_addr,
    sizeof(server_addr));
  if (retcode < 0)
  {
    printf("*** ERROR - connect() failed \n");
    exit(-1);
  }

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

  // Read and send the file to the receiver
  do
  {
    length = read(fh, out_buf, SIZE);
    if (length > 0)
    {
      retcode = send(client_s, out_buf, length, 0);
      if (retcode < 0)
      {
        printf("*** ERROR - recv() failed \n");
        exit(-1);
      }
    }
  } while (length > 0);

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

int listen(){
  int listen_sock, err, i, bytes, len, client_sock, client_addr_size;
  char msg[BUFFER_SIZE];

  struct sockaddr_in server_addr, client_addr;

  pid_t child;

  signal(SIGTERM, cleanExit);
  signal(SIGINT, cleanExit);

  child = fork();


  if (child == 0) {

      /***** udp *****/

      listen_sock = socket(AF_INET, SOCK_DGRAM, 0);
      if (listen_sock < 0) {
          perror("Unable to create udp socket\n");
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
          printf("Unable to bind udp socket\n");
          return EXIT_ERROR;
      }

      client_addr_size = sizeof(struct sockaddr_in);

      while (-1) {
          bytes = recvfrom(listen_sock, &msg, BUFFER_SIZE, 0, (struct sockaddr *) &client_addr, &client_addr_size);

          if (bytes <= 0) {
              /* Error or connection closed */
              break;
          }

          printf("Received %d bytes from udp client\n", bytes);
          printf("%s", msg);

          printf("Enter message for udp client (type 'close' to end communication):\n");
          fgets(msg, BUFFER_SIZE, stdin);

          len = strlen(msg);

          bytes = sendto(listen_sock, &msg, len, 0, (struct sockaddr *) &client_addr, sizeof(struct sockaddr_in));
          printf("Sent %d bytes to udp client\n", bytes);

          if (bytes <= 0) {
              /* Error or connection closed */
              break;
          }

          err = strcmp(msg, EXIT_NOTIFICATION);

          if (err == 0) {
              printf("Closing udp client socket\n");
              shutdown(listen_sock, 2);
              return EXIT_SUCCESS;
          }
      }
  }

  /***** tcp *****/

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
      perror("Unable to listen socket\n");
      return EXIT_ERROR;
  }

  /* We are now connected */
  client_addr_size = sizeof(struct sockaddr_in);

  client_sock = accept(listen_sock, (struct sockaddr *) &client_addr, &client_addr_size);

  if (client_sock < 0) {
      perror("accept() failed\n");
      return EXIT_ERROR;
  }

  printf("Serving new tcp client\n");

  while (-1) {
      bytes = recv(client_sock, &msg, BUFFER_SIZE, 0);

      if (bytes <= 0) {
          /* Error or connection closed */
          break;
      }

      printf("Received %d bytes from tcp client\n", bytes);

      printf("%s", msg);

      printf("Enter message for tcp client (type 'close' to end communication):\n");

      fgets(msg, BUFFER_SIZE, stdin);

      len = strlen(msg);

      bytes = send(client_sock, &msg, len, 0);

      printf("Sent %d bytes to tcp client\n", bytes);

      if (bytes <= 0) {
          /* Error or connection closed */
          break;
      }

      err = strcmp(msg, EXIT_NOTIFICATION);

      if (err == 0) {
          printf("Closing tcp client socket\n");
          shutdown(listen_sock, 2);
          return EXIT_SUCCESS;
      }
  }
}
