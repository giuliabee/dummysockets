#ifdef _WIN32
#include <windows.h>
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


int main( int argc, char * argv[])
{
    SOCKET hSocket; struct sockaddr_in addr;
    int nPort = 0;
#ifdef _WIN32
    WSADATA stack_info;
    
    WSAStartup(MAKEWORD(2,0), &stack_info );
#endif

    if( argc > 1 )
    {
	nPort = atoi( argv[1] );
    }
    if( nPort == 0 )
    {
	printf( "Usage: %s <udpport>\nUsing default=1812\n", argv[0] );
	nPort = 1812;
    }
    

    // socket
    hSocket = socket( PF_INET, SOCK_DGRAM, 0 );
    if( hSocket == INVALID_SOCKET )
    {
	printf( "socket() error %d\n", SOCKET_ERRNO );
	return -1;
    }
    
    // bind
    addr.sin_family = AF_INET ;
    addr.sin_addr.s_addr = htonl (INADDR_ANY);
    addr.sin_port = htons ((unsigned short)nPort );
    if ( bind( hSocket, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR )
    {
	printf( "bind() error %d\n", SOCKET_ERRNO );
	return -1;
    }
    
    // loop forever
    for(;;)
    {
	struct sockaddr udpaddrfrom; int fromlen = sizeof(udpaddrfrom);
	char Buffer[1024]; int cbRead;	
	
	// Get an udp packet
	cbRead = recvfrom( hSocket, Buffer, sizeof(Buffer), 0, &udpaddrfrom, &fromlen  );
	if( cbRead <= 0 )
	{
	    printf( "recvfrom() error %d\n", SOCKET_ERRNO );
	    //continue;
	    return -1;
	}

	// asciiZ string to print it
	Buffer[cbRead] = 0;
	printf( "recvfrom()=>cbRead=%d, Buffer=%s\n", cbRead, Buffer );
    }
    return 0;
}