#if defined (WIN32)
    #include <winsock2.h>
    typedef int socklen_t;
#elif defined (linux)
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #define closesocket(s) close(s)
    typedef int SOCKET;
    typedef struct sockaddr_in SOCKADDR_IN;
    typedef struct sockaddr SOCKADDR;
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
void cleanExit(){exit(0);}

#define EXIT_SUCCESS    0
#define EXIT_ERROR      1

#define SERVER_PORT 12345
#define BUFFER_SIZE 280

#define EXIT_NOTIFICATION   "close\n"


int main() {
    int listen_sock, err, i, bytes, len, client_sock, client_addr_size;
    char msg[BUFFER_SIZE];

    struct sockaddr_in serv_addr, client_addr;

    pid_t child;

    signal(SIGTERM, cleanExit);
    signal(SIGINT, cleanExit);

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
        err = bind(listen_sock, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
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

            printf("Enter message for udp client (type 'close' to end communication):\n");
            fgets(msg, BUFFER_SIZE, stdin);

            len = strlen(msg);

            bytes = sendto(listen_sock, &msg, len, 0, (struct sockaddr *) &client_addr, sizeof(struct sockaddr_in));
            printf("Sent %d bytes to udp client\n", bytes);

            if (bytes <= 0) {
                /* Error or connection closed */
                break;
            }

            err = strcmp(msg, EXIT_NOTIFICATION);

            if (err == 0) {
                printf("Closing udp client socket\n");
                shutdown(listen_sock, 2);
                return EXIT_SUCCESS;
            }
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
    err = bind(listen_sock, (struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
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

    client_sock = accept(listen_sock, (struct sockaddr *) &client_addr, &client_addr_size);

    if (client_sock < 0) {
        perror("accept() failed\n");
        return EXIT_ERROR;
    }

    printf("Serving new tcp client\n");

    while (-1) {
        bytes = recv(client_sock, &msg, BUFFER_SIZE, 0);

        if (bytes <= 0) {
            /* Error or connection closed */
            break;
        }

        printf("Received %d bytes from tcp client\n", bytes);

        printf("%s", msg);

        printf("Enter message for tcp client (type 'close' to end communication):\n");

        fgets(msg, BUFFER_SIZE, stdin);

        len = strlen(msg);

        bytes = send(client_sock, &msg, len, 0);

        printf("Sent %d bytes to tcp client\n", bytes);

        if (bytes <= 0) {
            /* Error or connection closed */
            break;
        }

        err = strcmp(msg, EXIT_NOTIFICATION);

        if (err == 0) {
            printf("Closing tcp client socket\n");
            shutdown(listen_sock, 2);
            return EXIT_SUCCESS;
        }
    }
}

