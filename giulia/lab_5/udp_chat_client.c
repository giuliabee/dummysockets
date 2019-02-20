#include <sys/types.h>  /* for constants (e.g. AF_INET) */
#include <sys/socket.h>        /* for socket(), send(), connect(), recv(), ... */

#include <arpa/inet.h>        /* for inet_pton(), htons() */
#include <stdio.h>        /* for snprintf(), printf() */
#include <string.h>        /* for memset(), strlen() */

#define EXIT_SUCCESS 0
#define EXIT_ERROR   1

#define EXIT_NOTIFICATION "close\n"

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT    12345

#define BUFFER_SIZE 512

int main() {
    int sock, err, i, bytes, len;
    char msg[BUFFER_SIZE];

    struct sockaddr_in dest_addr;

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

