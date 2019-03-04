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

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_SIZE 280

#define EXIT_NOTIFICATION   "close\n"

int main() {
    int sock, err, i, bytes, len;
    char msg[BUFFER_SIZE];

    struct sockaddr_in dest_addr;

    signal(SIGTERM, cleanExit);
    signal(SIGINT, cleanExit);

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("Unable to create socket");
        return EXIT_ERROR;
    }

    /* initialize address */
    memset((void *) &dest_addr, 0, sizeof(dest_addr));    /* clear server address */
    dest_addr.sin_family = AF_INET;                       /* address type is INET */
    dest_addr.sin_port = htons(SERVER_PORT);

    /* Convert the address from strin gform to binary form */
    err = inet_pton(AF_INET, SERVER_ADDRESS, &dest_addr.sin_addr);
    if (err <= 0) {
        perror("Address creation error");
        return EXIT_ERROR;
    }

    while (-1) {
        printf("Enter message: \n");

        fgets(msg, BUFFER_SIZE, stdin);

        len = strlen(msg);

        sendto(sock, &msg, len, 0, (struct sockaddr *) &dest_addr, sizeof(struct sockaddr_in));

        printf("Sent %d bytes\n", len);

        bytes = recv(sock, msg, BUFFER_SIZE, 0);

        if (bytes < 0) {
            perror("Failed to receive");
            return EXIT_ERROR;
        }

        printf("Received %d bytes: %s", bytes, msg);

        err = strcmp(msg, EXIT_NOTIFICATION);

        if (err == 0) {
            printf("Server closed udp socket\n");
            return EXIT_SUCCESS;
        }
    }
}

