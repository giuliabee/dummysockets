#if defined (WIN32) //CLIENT CELUI QUI COMMENCE A RECEVOIR
    #include <winsock2.h>
    typedef int socklen_t;
#elif defined (linux)
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define INVALID_SOCKET -1
    #define SOCKET_ERROR -1
    #define closesocket(s) close(s)
    typedef int SOCKET;
    typedef struct sockaddr_in SOCKADDR_IN;
    typedef struct sockaddr SOCKADDR;
#endif
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PORT 23

SOCKET sock;
SOCKADDR_IN sin;
SOCKADDR_IN csin;
SOCKET csock;
char buffer[200];
int sock_err; 
socklen_t recsize = sizeof(csin);
 
int main(void)
{
	
	char answer = 'y';
	
	#if defined (WIN32)
        WSADATA WSAData;
        int erreur = WSAStartup(MAKEWORD(2,2), &WSAData);
    #else
        int erreur = 0;
    #endif
 
    /* Si les sockets Windows fonctionnent */
    if(!erreur)
    {
        /* Création de la socket */
        sock = socket(AF_INET, SOCK_STREAM, 0);
		do
		{
		Reception_of_msg();
		Send_a_msg();
		printf("Do you want to stop the chat ? y / n \n"); 
		scanf("%c",&answer);
		}while(answer !='y');
		
	}
	
	/* Il ne faut pas oublier de fermer la connexion (fermée dans les deux sens) */
		shutdown(csock, 2);
	
	/* On ferme la socket */
        closesocket(sock);
 
        #if defined (WIN32)
            WSACleanup();
        #endif
    
 
    /* On attend que l'utilisateur tape sur une touche, puis on ferme */
    getchar();
    return EXIT_SUCCESS;
}

void Send_a_msg()
{
        /* Si la socket est valide*\
        if(sock != INVALID_SOCKET)
        {
            printf("La socket %d est maintenant ouverte en mode TCP/IP\n", sock);
 
            /* Configuration */
            sin.sin_addr.s_addr    = htonl(INADDR_ANY);   /* Adresse IP automatique */
            sin.sin_family         = AF_INET;             /* Protocole familial (IP) */
            sin.sin_port           = htons(PORT);         /* Listage du port */
            sock_err = bind(sock, (SOCKADDR*)&sin, sizeof(sin));
 
            /* Si la socket fonctionne */
            if(sock_err != SOCKET_ERROR)
            {
                /* Démarrage du listage (mode server) */
                sock_err = listen(sock, 5);
                printf("Listage du port %d...\n", PORT);
 
                /* Si la socket fonctionne */
                if(sock_err != SOCKET_ERROR)
                {
                    /* Attente pendant laquelle le client se connecte */
                    printf("Patientez pendant que le client se connecte sur le port %d...\n", PORT);        
 
                    csock = accept(sock, (SOCKADDR*)&csin, &recsize);
                    printf("Un client se connecte avec la socket %d de %s:%d\n", csock, inet_ntoa(csin.sin_addr), htons(csin.sin_port));
					printf("Quel est le mot (message) que vous voulez envoyer ? ");
						scanf("%s", &buffer);
						sock_err = send(csock, buffer, sizeof(buffer), 0);
 
                    if(sock_err != SOCKET_ERROR)
					{
						printf("Chaine envoyée : %s\n", buffer);
					}
                        
                    else
						{
							printf("Erreur de transmission\n");
							shutdown(csock, 2);
						}
                }
            }
 
		

	
        /* Si la socket est valide */
        if(sock != INVALID_SOCKET)
        {
            printf("La socket %d est maintenant ouverte en mode TCP/IP\n", sock);
 
            /* Configuration */
            sin.sin_addr.s_addr    = htonl(INADDR_ANY);   /* Adresse IP automatique */
            sin.sin_family         = AF_INET;             /* Protocole familial (IP) */
            sin.sin_port           = htons(PORT);         /* Listage du port */
            sock_err = bind(sock, (SOCKADDR*)&sin, sizeof(sin));
 
            /* Si la socket fonctionne */
            if(sock_err != SOCKET_ERROR)
            {
                /* Démarrage du listage (mode server) */
                sock_err = listen(sock, 5);
                printf("Listage du port %d...\n", PORT);
 
                /* Si la socket fonctionne */
                if(sock_err != SOCKET_ERROR)
                {
                    /* Attente pendant laquelle le client se connecte */
                    printf("Patientez pendant que le client se connecte sur le port %d...\n", PORT);        
 
                    csock = accept(sock, (SOCKADDR*)&csin, &recsize);
                    printf("Un client se connecte avec la socket %d de %s:%d\n", csock, inet_ntoa(csin.sin_addr), htons(csin.sin_port));
					printf("Quel est le mot (message) que vous voulez envoyer ? ");
						scanf("%s", &buffer);
						sock_err = send(csock, buffer, sizeof(buffer), 0);
 
                    if(sock_err != SOCKET_ERROR)
					{
						printf("Chaine envoyée : %s\n", buffer);
					}
                        
                    else
						{
							printf("Erreur de transmission\n");
							//shutdown(csock, 2);
						}
                }
            }
 
		}

	#if defined (WIN32)
        WSADATA WSAData;
        int erreur = WSAStartup(MAKEWORD(2,2), &WSAData);
    #else
        int erreur = 0;
    #endif
 
    SOCKET sock;
    SOCKADDR_IN sin;
    SOCKET csock;
    SOCKADDR_IN csin;
	char buffer[200];
	//printf("Quel est le mot (message) que vous voulez envoyer ? ");
    //scanf("%s", &buffer);
    socklen_t recsize = sizeof(csin);
    int sock_err;
 
    /* Si les sockets Windows fonctionnent */
    if(!erreur)
    {
        sock = socket(AF_INET, SOCK_STREAM, 0);
 
        /* Si la socket est valide */
        if(sock != INVALID_SOCKET)
        {
            printf("La socket %d est maintenant ouverte en mode TCP/IP\n", sock);
 
            /* Configuration */
            sin.sin_addr.s_addr    = htonl(INADDR_ANY);   /* Adresse IP automatique */
            sin.sin_family         = AF_INET;             /* Protocole familial (IP) */
            sin.sin_port           = htons(PORT);         /* Listage du port */
            sock_err = bind(sock, (SOCKADDR*)&sin, sizeof(sin));
 
            /* Si la socket fonctionne */
            if(sock_err != SOCKET_ERROR)
            {
                /* Démarrage du listage (mode server) */
                sock_err = listen(sock, 5);
                printf("Listage du port %d...\n", PORT);
 
                /* Si la socket fonctionne */
                if(sock_err != SOCKET_ERROR)
                {
                    /* Attente pendant laquelle le client se connecte */
                    printf("Patientez pendant que le client se connecte sur le port %d...\n", PORT);        
 
                    csock = accept(sock, (SOCKADDR*)&csin, &recsize);
                    printf("Un client se connecte avec la socket %d de %s:%d\n", csock, inet_ntoa(csin.sin_addr), htons(csin.sin_port));
					printf("Quel est le mot (message) que vous voulez envoyer ? ");
						scanf("%s", &buffer);
						sock_err = send(csock, buffer, sizeof(buffer), 0);
 
                    if(sock_err != SOCKET_ERROR)
					{
						printf("Chaine envoyée : %s\n", buffer);
					}
                        
                    else
						{
							printf("Erreur de transmission\n");
							shutdown(csock, 2);
						}
                    /* Il ne faut pas oublier de fermer la connexion (fermée dans les deux sens) */
                    shutdown(csock, 2);
                }
            }
 
            /* Fermeture de la socket */
            printf("Fermeture de la socket...\n");
            closesocket(sock);
            printf("Fermeture du serveur terminee\n");
        }
 
        #if defined (WIN32)
            WSACleanup();
        #endif
    }
 
    /* On attend que l'utilisateur tape sur une touche, puis on ferme */
    getchar();
}

void Reception_of_msg()
{

        /* Configuration de la connexion */
        sin.sin_addr.s_addr = inet_addr("127.0.0.1");
        sin.sin_family = AF_INET;
        sin.sin_port = htons(PORT);
		
		
 
        /* Si l'on a réussi à se connecter */
        if(connect(sock, (SOCKADDR*)&sin, sizeof(sin)) != SOCKET_ERROR)
        {
            printf("Connection à %s sur le port %d\n", inet_ntoa(sin.sin_addr), htons(sin.sin_port));
            
            /* Si l'on reçoit des informations : on les affiche à l'écran */
            if(recv(sock, buffer, sizeof(buffer), 0) != SOCKET_ERROR)
			{printf("Recu : %s \n", buffer);}
			
        }
        /* sinon, on affiche "Impossible de se connecter" */
        else
        {
            printf("Impossible de se connecter\n");
        }
 
        
}

