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
#endif

#include "sw.h"

//----- Defines ---------------------------------------------------------------
#define  PORT_NUM     6009          // Arbitrary port number for the server
#define  RECV_FILE  "recvFile.dat"  // File name of received file

typedef struct
{
    int socket;
    struct sockaddr *addr;
    socklen_t addr_len;
} transport_info_t;

static int receive_data(void *buffer, int buffer_size, void *ctx)
{
    int length;
    transport_info_t *transport = (transport_info_t *)ctx;
	
		length = recvfrom(transport->socket, buffer, buffer_size, 0, transport->addr,	&(transport->addr_len));
		
		
    

    return length;
}

static int send_data(void *data, int data_len, void *ctx)
{
    int ret_code;
    transport_info_t *transport = (transport_info_t *)ctx;
	
		ret_code = sendto(transport->socket, data, data_len, 0, transport->addr,	transport->addr_len);
		
    

    return ret_code;
}

static int write_data_to_file(void *data, int data_len, void *ctx)
{
    int length, fd = *(int *)ctx;

    length = write(fd, data, data_len);

    return length;
}

//----- Prototypes ------------------------------------------------------------
int recvFile(char *fileName, int portNum);

//===== Main program ==========================================================
int main()
{
  int                  portNum;         // Port number to receive on
  int                  retcode;         // Return code

  // Initialize parameters
  portNum= PORT_NUM;

  // Receive the file
  printf("Starting file reception... \n");
  retcode = recvFile(RECV_FILE, portNum);
  if (retcode)
      return -1;
  printf("File received. It's name is \n" + RECV_FILE);

  // Return
  return(0);
}

int recvFile(char *fileName, int portNum)
{
#ifdef WIN
  WORD wVersionRequested = MAKEWORD(1,1);       // Stuff for WSA functions
  WSADATA wsaData;                              // Stuff for WSA functions
#endif
  int                  server_s;        // Welcome socket descriptor
  struct sockaddr_in   server_addr;     // Server Internet address
  int                  fh;              // File handle
  int                  retcode;         // Return code
  transport_info_t transport = {};

#ifdef WIN
  // This stuff initializes winsock
  WSAStartup(wVersionRequested, &wsaData);
#endif

  // Create a socket
  server_s = socket(AF_INET, SOCK_DGRAM, 0);
  if (server_s < 0)
  {
    printf("*** ERROR - socket() failed \n");
    exit(-1);
  }
  
  // Fill-in my socket's address information
  server_addr.sin_family = AF_INET;                 // Address family to use
  server_addr.sin_port = htons(PORT_NUM);           // Port number to use
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);  // Listen on any IP address
  
  transport.addr = (struct sockaddr *)&server_addr;
  transport.addr_len = sizeof(struct sockaddr);
  transport.socket = server_s;
  
  retcode = bind(server_s, (struct sockaddr *)&server_addr,
    sizeof(server_addr));
  if (retcode < 0)
  {
    printf("*** ERROR - bind() failed \n");
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

  sw_ctx_t *sw_ctx = sw_init(10);
  
  sw_set_cb(sw_ctx, SW_RECV_CB, receive_data, &transport);
  sw_set_cb(sw_ctx, SW_SEND_CB, send_data, &transport);
  sw_set_cb(sw_ctx, SW_WRITE_DATA_CB, write_data_to_file, &fh);
  
  sw_recv_file(sw_ctx);
  
  sw_deinit(sw_ctx);
  // >>> Step #6 <<<
  // Close all open sockets
#ifdef WIN
  retcode = closesocket(server_s);
  if (retcode < 0)
  {
    printf("*** ERROR - closesocket() failed \n");
    exit(-1);
  }
#endif
#ifdef BSD
  retcode = close(server_s);
  if (retcode < 0)
  {
    printf("*** ERROR - close() failed \n");
    exit(-1);
  }
#endif

#ifdef WIN
  // This stuff cleans-up winsock
  WSACleanup();
#endif

  // Return zero and terminate
  return(0);
}