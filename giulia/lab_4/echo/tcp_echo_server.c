#include <sys/types.h>  /* for constants (e.g. AF_INET) */
#include <sys/socket.h>        /* for socket(), send(), connect(), recv(), ... */

#include <arpa/inet.h>        /* for inet_pton(), htons() */
#include <stdio.h>        /* for snprintf(), printf() */
#include <string.h>        /* for memset(), strlen() */

#define EXIT_SUCCESS 0
#define EXIT_ERROR   1

#define SERVER_PORT    12345

#define BUFFER_SIZE 128

void serve_client(int client_sock) {

        int bytes, i=0;
        char msg[BUFFER_SIZE];

        do {
                bytes = recv(client_sock, msg+i, 1, 0);

                if (bytes <= 0) { 
                        /* Error or connection closed */
                        break;
                }

                if (msg[i] == '\n') {
                        printf("Received %d bytes\n", i);
                        bytes = send(client_sock, msg, i+1, 0);
                        printf("Sent %d bytes\n", bytes);
                        i=0;
                        continue;
                }
                                
                i++;

                if ( i + 1 >= BUFFER_SIZE ) {
                        /* Avoid a buffer overflow */
                        perror("Buffer overflow");
                        break;
                }


        } while (bytes > 0);

        printf("Closing client socket\n");
        shutdown(client_sock, 2);

}

int main() {
        int listen_sock, err, i, bytes, client_sock, client_addr_size;

        struct sockaddr_in serv_addr, client_addr;

        listen_sock = socket(AF_INET, SOCK_STREAM, 0);
        if (listen_sock < 0) {
                perror("Unable to create socket");
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
                perror("Unable to bind socket"); 
                return EXIT_ERROR; 
        }

        err = listen(listen_sock, 10);
        if (err < 0) {
                perror("Unable to listen socket"); 
                return EXIT_ERROR; 
        }

        /* We are now connected */

        while (-1) {
                client_addr_size = sizeof(struct sockaddr_in);
                client_sock = accept(listen_sock, (struct sockaddr *) &client_addr, &client_addr_size );

                if (client_sock < 0) {
                        perror("accept() failed");
                        continue;
                }

                printf("Serving new client\n");

                serve_client(client_sock);
        }


        return EXIT_SUCCESS;
}

