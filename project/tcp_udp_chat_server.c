#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define EXIT_SUCCESS    0
#define EXIT_ERROR      1

#define SERVER_PORT 12345
#define BUFFER_SIZE 280

#define NUM_THREADS 50

#define EXIT_NOTIFICATION   "close\n"

void *serve_udp_client(void *client_sock) {
    int client_addr_size, sock, bytes, len, err;

    char msg[BUFFER_SIZE];

    struct sockaddr_in client_addr;

    sock = (long) client_sock;

    client_addr_size = sizeof(struct sockaddr_in);

    while (1) {
        bytes = recvfrom(sock, &msg, BUFFER_SIZE, 0, (struct sockaddr *) &client_addr, &client_addr_size);

        if (bytes <= 0) {
            /* Error or connection closed */
            break;
        }

        printf("Received %d bytes from udp client %d\n", bytes, sock);
        printf("%s", msg);

        printf("Enter message for udp client %d (type 'close' to end communication):\n", sock);
        fgets(msg, BUFFER_SIZE, stdin);

        len = strlen(msg);

        bytes = sendto(sock, &msg, len, 0, (struct sockaddr *) &client_addr, client_addr_size);
        printf("Sent %d bytes to udp client %d\n", bytes, sock);

        if (bytes <= 0) {
            /* Error or connection closed */
            break;
        }

        err = strcmp(msg, EXIT_NOTIFICATION);

        if (err == 0) {
            printf("Ending udp communication with client %d\n", sock);
            return EXIT_SUCCESS;
        }
    }
}

void *serve_tcp_client(void *client_sock) {
    int sock, bytes, len, err;

    char msg[BUFFER_SIZE];

    struct sockaddr_in client_addr;

    sock = (long) client_sock;

    while (1) {
        bytes = recv(sock, &msg, BUFFER_SIZE, 0);

        if (bytes <= 0) {
            /* Error or connection closed */
            break;
        }

        printf("Received %d bytes from tcp client %d\n", bytes, sock);

        printf("%s", msg);

        printf("Enter message for tcp client %d (type 'close' to end communication):\n", sock);

        fgets(msg, BUFFER_SIZE, stdin);

        len = strlen(msg);

        bytes = send(sock, &msg, len, 0);

        printf("Sent %d bytes to tcp client %d\n", bytes, sock);

        if (bytes <= 0) {
            /* Error or connection closed */
            break;
        }

        err = strcmp(msg, EXIT_NOTIFICATION);

        if (err == 0) {
            printf("Closing tcp client socket %d\n", sock);
            shutdown(sock, 2);
            return EXIT_SUCCESS;
        }
    }
}

int main() {
    int listen_sock, client_sock, client_addr_size, err, i;

    struct sockaddr_in server_addr, client_addr;

    pthread_t thread;

    pid_t child;

    child = fork();


    if (child == 0) {

        /***** udp *****/

        client_sock = socket(AF_INET, SOCK_DGRAM, 0);
        if (client_sock < 0) {
            perror("Unable to create udp socket\n");
            return EXIT_ERROR;
        }

        /* initialize address */
        memset((void *) &server_addr, 0, sizeof(server_addr));    /* clear server address */
        server_addr.sin_family = AF_INET;                       /* address type is INET */
        server_addr.sin_port = htons(SERVER_PORT);
        server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        /* Bind the port number to my process */
        err = bind(client_sock, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in));
        if (err < 0) {
            printf("Unable to bind udp socket\n");
            return EXIT_ERROR;
        }

        while (1) {
            printf("Serving new udp client\n");

            pthread_create(&thread, NULL, serve_udp_client, (void *) client_sock);
        }

    } else {

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
            perror("Unable to listen tcp socket\n");
            return EXIT_ERROR;
        }
        /* We are now connected */

        while (1) {
            client_addr_size = sizeof(struct sockaddr_in);

            client_sock = accept(listen_sock, (struct sockaddr *) &client_addr, &client_addr_size);

            if (client_sock < 0) {
                perror("accept() failed\n");
                continue;
            }

            printf("Serving new tcp client\n");

            pthread_create(&thread, NULL, serve_tcp_client, (void *) client_sock);
        }

    }
}

