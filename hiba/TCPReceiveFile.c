#define  WIN               // WIN for Winsock and BSD for BSD sockets

//----- Include files ---------------------------------------------------------
#include <stdio.h>          // Needed for printf()
#include <string.h>         // Needed for memcpy() and strcpy()
#include <stdlib.h>         // Needed for exit()
#include <fcntl.h>          // Needed for file i/o constants
#include <string.h>
#include <ctype.h>
#ifdef WIN
  #include <windows.h>      // Needed for all Winsock stuff
  #include <io.h>           // Needed for open(), close(), and eof()
  #include <sys\stat.h>     // Needed for file i/o constants
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
#endif

//----- Defines ---------------------------------------------------------------
#define  PORT_NUM    6069           // Arbitrary port number for the server
#define  SIZE        256            // Buffer size
#define  RECV_FILE  "recvFile.dat"  // File name of received file

//----- Prototypes ------------------------------------------------------------
int recvFile(char *fileName, int portNum, int maxSize, int options);

//===== Main program ==========================================================
int main()
{
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

//=============================================================================
//=  Function to receive a file using TCP                                     =
//=============================================================================
//=  Inputs:                                                                  =
//=    fileName -- Name of file to create and write                           =
//=    portNum --- Port number to listen and receive on                       =
//=    maxSize --- Maximum size in bytes for written file (not implemented)   =
//=    options --- Options (not implemented)                                  =
//=---------------------------------------------------------------------------=
//=  Outputs:                                                                 =
//=    Returns -1 for fail and 0 for success                                  =
//=---------------------------------------------------------------------------=
//=  Side effects:                                                            =
//=    None known                                                             =
//=---------------------------------------------------------------------------=
//=  Bugs:                                                                    =
//=    None known                                                             =
//=---------------------------------------------------------------------------=
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