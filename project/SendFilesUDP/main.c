#define  WIN               // WIN for Winsock and BSD for BSD sockets

//----- Include files ---------------------------------------------------------
#include <stdio.h>          // Needed for printf()
#include "sqlite3.h"
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
#define  PORT_NUM    6069   // Port number used at the server
#define  SIZE        256    // Buffer size

//----- Prototypes ------------------------------------------------------------
int sendFile(char *fileName, char *destIpAddr, int destPortNum, int options);
int callback(void *NotUsed, int argc, char **argv,char **azColName)
{
        NotUsed = 0;
    for (int i = 0; i < argc; i++) {
        printf(" %s = %s", azColName[i], argv[i] ? argv[i] : "NULL");

     }
    printf("\n");
    return 0;
}

//===== Main program ==========================================================
int main(int argc, char *argv[])
{
  char                 sendFileName[256];   // Send file name
  char                 recv_ipAddr[16];     // Reciver IP address
  int                  recv_port;           // Receiver port number
  int                  options;             // Options
  int                  retcode;             // Return code

     sqlite3 *db;
    char * name_database = "Login.db";
    char *err_msg = 0;
    char *sql;   //requete SQL

    int step ;   //

    int rc = sqlite3_open("Login.db", &db);
    printf("\nRC open Database = %d\n",rc);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }
    sql = "DROP TABLE IF EXISTS Login;"
                "CREATE TABLE Login(Id INT, Name TEXT, Password INT);"
                "INSERT INTO Login VALUES(1, 'Imad', 1111);"
                "INSERT INTO Login VALUES(2, 'Hiba', 2222);"
                "INSERT INTO Login VALUES(3, 'Jiulia', 3333);"
                "INSERT INTO Login VALUES(4, 'Thomas', 4444);";
    rc = sqlite3_exec(db, sql, 0, 0, &err_msg);
  //  printf("\nRC sqltite_exec = %d -> %s \n\n",rc,sql);
    if (rc != SQLITE_OK ) {
        fprintf(stderr, "SQL error: %s\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    } else {
        fprintf(stdout, "Table Login created successfully\n");
    }
    int last_id = sqlite3_last_insert_rowid(db);
    printf("The last Id of the inserted row is %d\n", last_id);
    printf("Entrez un nom : ");
    char* name;
    gets(name);
    printf("Entrez un mot de pass : ");
    char mdp[]="";
    gets(mdp);
    int password=0;
    sql = "SELECT Password FROM Login WHERE Name=@names";

    sqlite3_stmt *res;
    rc = sqlite3_prepare_v2(db, sql, -1, &res, 0);

    if (rc == SQLITE_OK) {
        int idx = sqlite3_bind_parameter_index(res, "@names");
        rc=sqlite3_bind_text(res, idx, name, -1, SQLITE_STATIC);     // the string is static
        printf("\nRC sqltite_exec = %d -> %s\n",rc,sql);

     } else {
        fprintf(stderr, "Failed to execute statement: %s\n", sqlite3_errmsg(db));
    }
    while ((step = sqlite3_step(res) == SQLITE_ROW)) {


  if (atoi(sqlite3_column_text(res, 0))==atoi(mdp))
  {
  password =1;
  }

     //   printf("%s: ", sqlite3_column_text(res, 0));


     //   printf("%s\n", sqlite3_column_text(res, 1));

    }


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



if(password==1)
{
     retcode = sendFile(sendFileName, recv_ipAddr, recv_port, options);
  printf("File transfer is complete \n");

}else{printf("Error mdp  \n");}
sqlite3_finalize(res);
    sqlite3_close(db);
    return 0;
  // Return
  return(0);
}

//=============================================================================
//=  Function to send a file using TCP                                        =
//=============================================================================
//=  Inputs:                                                                  =
//=    fileName ----- Name of file to open, read, and send                    =
//=    destIpAddr --- IP address or receiver                                  =
//=    destPortNum -- Port number receiver is listening on                    =
//=    options ------ Options (not implemented)                               =
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




//sudo apt-get install sqlite3 libsqlite3-dev
//linker -lsqlite3 -std=c99

