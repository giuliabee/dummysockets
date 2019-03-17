#define  WIN
#ifdef _WIN32
    #include <windows.h>      // Needed for all Winsock stuff
    #include <io.h>           // Needed for open(), close(), and eof()
    #include <sys\stat.h>     // Needed for file i/o constants
    #include <winsock2.h>
    #include <ws2tcpip.h>
    typedef int socklen_t;
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <netinet/in.h>   // Needed for sockets stuff
    #include <netdb.h>        // Needed for sockets stuff
    #include <sys/io.h>       // Needed for open(), close(), and eof()
    #include <sys/stat.h>     // Needed for file i/o constants
    #include <resolv.h>
    #define closesocket(s) close(s)
    typedef int SOCKET;
    typedef struct sockaddr_in SOCKADDR_IN;
    typedef struct sockaddr SOCKADDR;
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>          // Needed for file i/o constants
#include <ctype.h>

void cleanExit() { exit(0); }

#define  PORT_NUM    6069   // Port number used at the server
#define  SIZE        280    // Buffer size

#define EXIT_SUCCESS    0
#define EXIT_ERROR      1

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_SIZE 280

#define EXIT_NOTIFICATION   "close\n"

int sendFile(char *fileName, char *destIpAddr, int destPortNum, int options);

int main() {
//Chat();
//TransferFile();
}

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

int TransferFile(int argc, char *argv[]){
  char                 sendFileName[256];   // Send file name
  char                 recv_ipAddr[16];     // Reciver IP address
  int                  recv_port;           // Receiver port number
  int                  options;             // Options
  int                  retcode;             // Return code

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
  options = 0;     // This parameter is unused in this implementation

  // Send the file
  printf("Starting file transfer... \n");
  retcode = sendFile(sendFileName, recv_ipAddr, recv_port, options);
  printf("File transfer is complete \n");

  // Return
  return(0);
}

int Chat(){
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
      ListCommandLines();
      printf("Enter message: \n");

      fgets(msg, BUFFER_SIZE, stdin);

      len = strlen(msg);

      bytes = send(sock, msg, len, 0);

      if (bytes < 0) {
          perror("Failed to send");
          return EXIT_ERROR;
      }

      printf("Sent %d bytes\n", bytes);


    //  action(sock);

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
/*
void action(int msg){
  if(strncmp("exit", msg, 4) == 0)
  {
    printf("test");
    exit(EXIT_SUCCESS);
  }
  else if(strncmp("help", msg, 4) == 0)
		{
            send(msg, ListCommandLines(), sizeof(BUFFER_SIZE), 0);
        }

        else if(strncmp("listf", BUFFER_SIZE, 5) == 0)
		{
            send(msg,"listf", sizeof(BUFFER_SIZE), 0);
        }

        else if(strncmp("trfd", BUFFER_SIZE, 4) == 0)
		{
            send(msg,"trfd", sizeof(BUFFER_SIZE), 0);
        }

        else if(strncmp("trfu", buff, 4) == 0)
		{
            send(msg, "trfu", sizeof(BUFFER_SIZE), 0);
        }

        else if(strncmp("private", BUFFER_SIZE, 7) == 0)
		{
            k = 1;
            send(msg, "private", sizeof(BUFFER_SIZE, 0);
        }

        else if(strncmp("public", buff, 6) == 0)
		{
            k = 1;
            send(msg, "public", sizeof(BUFFER_SIZE), 0);
        }

        else if (strncmp("ring", BUFFER_SIZE, 4) == 0)
		{
            k = 1;
            send(msg, "ring", sizeof(BUFFER);
          }
        }
        */



int sendFile(char *fileName, char *destIpAddr, int destPortNum, int options)
{
  //printf("Entree dans la boucle sendfile");
#ifdef _WIN32
  WORD wVersionRequested = MAKEWORD(1,1);       // Stuff for WSA functions
  WSADATA wsaData;                              // Stuff for WSA functions
#endif
  int                  client_s;        // Client socket descriptor
  struct sockaddr_in   server_addr;     // Server Internet address
  char                 out_buf[4096];   // Output buffer for data
  int                  fh;              // File handle
  int                  length;          // Length of send buffer
  int                  retcode;         // Return code

#ifdef _WIN32
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
  //printf("creation socket");
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
  //printf("realisation de la connextion");
  // Open file to send
#ifdef _WIN32
    fh = open(fileName, O_RDONLY | O_BINARY, S_IREAD | S_IWRITE);

  #else
    fh = open(fileName, O_RDONLY, S_IREAD | S_IWRITE);
  #endif
  if (fh == -1)
  {
     printf("  *** ERROR - unable to open '%p' \n", sendFile);
     exit(1);
  }
  //printf("ouverture fichier et ecriture");
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
#ifdef _WIN32
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
  //printf("envoi du doc");
#ifdef _WIN32
  // Clean-up winsock
  WSACleanup();
#endif

  // Return zero
  return(0);
}
