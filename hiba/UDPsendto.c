#ifdef _WIN32
#include <windows.h>
#define sleep( a) Sleep( 1000 * (a) )
#define SOCKET_ERRNO	WSAGetLastError()
#else

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <inttypes.h>
#include <strings.h>
#define closesocket close
#define SOCKET int
#define SOCKET_ERRNO	errno

#endif
#include <stdio.h>


static struct sockaddr UdpFormatAdress( char * host, u_short port )
{
struct sockaddr_in addr;
struct sockaddr addrRet;
struct hostent FAR *lphost ;
u_long IP;
    
    
    memset((char*)&addr, 0, sizeof(addr));
    /*	Soit on fournit une adresse IP, soit on fournit un nom	*/
    if ((IP = inet_addr(host)) == (u_long)INADDR_NONE)
	    {
	    if ((lphost  = gethostbyname(host))==NULL)
		    {
            memset( (char * )&addrRet, 0, sizeof(addrRet) );
		    return  addrRet;
		    }
	    addr.sin_family = lphost->h_addrtype;
#ifdef _WIN16 
	    _fmemcpy (&addr.sin_addr, lphost->h_addr, lphost->h_length);
#else 
    	memcpy (&addr.sin_addr, lphost->h_addr, lphost->h_length);
#endif
        }
    else
	    {
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = IP;
        }

    /*  Port destination    */    
    addr.sin_port = htons((u_short)port );    

    memcpy( (char *)&addrRet, (char *)&addr, sizeof(addrRet) );
    return addrRet;
}


int main( int argc, char * argv[])
{
    SOCKET hSocket; struct sockaddr_in addr;
    struct sockaddr udpaddr;
    int nPort = 1812;
    char szHost[256] = "localhost";
#ifdef _WIN32
    WSADATA stack_info;
    
    WSAStartup(MAKEWORD(2,0), &stack_info );
#endif

    if( argc > 1 )
    {
	nPort = atoi( argv[1] );
    }
    if( argc > 2 )
    {
	strcpy( szHost, argv[2] );
    }
    if( nPort == 0 )
    {
	printf( "Usage %s <host> <port>\nUsing default=localhost 1812", argv[0] );
	nPort = 1812;
    }

    // socket
    hSocket = socket( PF_INET, SOCK_DGRAM, 0 );
    if( hSocket == INVALID_SOCKET )
    {
	printf( "socket() error %d\n", SOCKET_ERRNO );
	return -1;
    }
    
     // bind()
    addr.sin_family = AF_INET ;
    addr.sin_addr.s_addr = htonl (INADDR_ANY);
    addr.sin_port = htons ((unsigned short)0 );
    if ( bind( hSocket, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR )
    {
	printf( "bind() error %d\n", SOCKET_ERRNO );
	return -1;
    }
	
    // convert (host, port) to a struct sockaddr
    udpaddr = UdpFormatAdress( szHost, (u_short)nPort );
    
    // loop forever
    for(;;)
    {
	char Buffer[256]; int cbBuffer;
	static int s_cbEnvoi = 1;

	// Build a buffer to send
	cbBuffer = sprintf( Buffer, "Envoi '%0d'\n", s_cbEnvoi++ );
	
	// send it
	if( sendto( hSocket, Buffer, cbBuffer, 0, &udpaddr, sizeof(udpaddr) ) == SOCKET_ERROR )
	{
	    printf( "sendto() error %d\n", SOCKET_ERRNO );
	    return -1;
	}
	printf( "sendto() cbBuffer=%d\n", cbBuffer);
	
	// sleep() to slow it
	sleep(1);
    }
}