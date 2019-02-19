#include <sys/types.h>  /* for constants (e.g. AF_INET) */
#include <sys/socket.h>        /* for socket(), send(), connect(), recv(), ... */

#include <arpa/inet.h>        /* for inet_pton(), htons() */
#include <stdio.h>        /* for snprintf(), printf() */
#include <string.h>        /* for memset(), strlen() */

#define EXIT_SUCCESS 0
#define EXIT_ERROR   1

#define SERVER_PORT    12345

#define BUFFER_SIZE 512

int main() {
        int listen_sock, err, i, bytes, len, client_sock, client_addr_size;
        char msg[BUFFER_SIZE];

        struct sockaddr_in serv_addr, client_addr;
        
        pid_t child; 
        
        child = fork();
        
        if (child == 0) {
        
                /***** udp *****/
                
                listen_sock = socket(AF_INET, SOCK_DGRAM, 0);
                if (listen_sock < 0) {
                        perror("Unable to create udp socket\n");
                        return EXIT_ERROR;
                }
                
                /* initialize address */
                memset((void *) &serv_addr, 0, sizeof(serv_addr));    /* clear server address */ 
                serv_addr.sin_family = AF_INET;                       /* address type is INET */ 
                serv_addr.sin_port = htons(SERVER_PORT);       
                serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

                /* Bind the port number to my process */
                err = bind(listen_sock, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in) );
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
                
                        printf("Enter message for udp client:\n");
                        fgets(msg, BUFFER_SIZE, stdin);
                        len = strlen(msg);
        
                        bytes = sendto(listen_sock, &msg, len, 0, (struct sockaddr *) &client_addr, sizeof(struct sockaddr_in));
                        printf("Sent %d bytes to udp client\n", bytes);
                                
                        if (bytes <= 0) { 
                                /* Error or connection closed */
                                break;
                        }
                        
                        printf("Closing udp client socket\n");
                        shutdown(listen_sock, 2);
                }
        }
        
        /***** tcp *****/

        listen_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_sock < 0) {
                perror("Unable to create tcp socket\n");
                return EXIT_ERROR;
                
        }
        
        /* initialize address */ 
        memset((void *) &serv_addr, 0, sizeof(serv_addr));    /* clear server address */ 
        serv_addr.sin_family = AF_INET;                       /* address type is INET */ 
        serv_addr.sin_port = htons(SERVER_PORT);
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        /* Bind the port number to my process */
        err = bind(listen_sock, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in) );
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
               
        while (-1) {
                client_sock = accept(listen_sock, (struct sockaddr *) &client_addr, &client_addr_size );

                if (client_sock < 0) {
                        perror("accept() failed\n");
                        continue;
                }

                printf("Serving new tcp client\n");
                bytes = recv(client_sock, &msg, BUFFER_SIZE, 0);

                if (bytes <= 0) { 
                        /* Error or connection closed */
                        break;
                }
                
                printf("Received %d bytes from tcp client\n", bytes);
                printf("%s", msg);
                        
                printf("Enter message for tcp client:\n");
                fgets(msg, BUFFER_SIZE, stdin);
                len = strlen(msg);

                bytes = send(client_sock, &msg, len, 0);
                printf("Sent %d bytes to tcp client\n", bytes);
                
                if (bytes <= 0) { 
                        /* Error or connection closed */
                        break;
                }
                
                printf("Closing tcp client socket\n");
                shutdown(client_sock, 2);
        }

        return EXIT_SUCCESS;
}

