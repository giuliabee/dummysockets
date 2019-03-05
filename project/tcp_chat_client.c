#if defined (WIN32)
    #include <winsock2.h>
    #include <ws2tcpip.h>
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

void cleanExit() { exit(0); }

#define EXIT_SUCCESS    0
#define EXIT_ERROR      1

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT 12345
#define BUFFER_SIZE 280

#define EXIT_NOTIFICATION   "close\n"

int main() {
    int sock, err, bytes, len;
    char msg[BUFFER_SIZE];

    struct sockaddr_in dest_addr;

    signal(SIGTERM, cleanExit);
    signal(SIGINT, cleanExit);

    sock = socket(AF_INET, SOCK_STREAM, 0);
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

    err = connect(sock, (struct sockaddr *) &dest_addr, sizeof(dest_addr));
    if (err < 0) {
        perror("Error connecting to the server");
        return EXIT_ERROR;
    }

    /* We are now connected */
    while (-1) {
        printf("Enter message: \n");

        fgets(msg, BUFFER_SIZE, stdin);

        len = strlen(msg);

        bytes = send(sock, msg, len, 0);

        if (bytes < 0) {
            perror("Failed to send");
            return EXIT_ERROR;
        }

        printf("Sent %d bytes\n", bytes);

        bytes = recv(sock, msg, BUFFER_SIZE, 0);

        if (bytes < 0) {
            perror("Failed to receive");
            return EXIT_ERROR;
        }

        printf("Received %d bytes: %s", bytes, msg);

        err = strcmp(msg, EXIT_NOTIFICATION);

        if (err == 0) {
            printf("Server closed tcp socket\n");
            return EXIT_SUCCESS;
        }
    }
}

#if defined (WIN32)
//from https://stackoverflow.com/questions/15660203/inet-pton-identifier-not-found
int inet_pton(int af, const char *src, void *dst) {
    struct sockaddr_storage ss;
    int size = sizeof(ss);
    char src_copy[INET6_ADDRSTRLEN + 1];

    ZeroMemory(&ss, sizeof(ss));
    /* stupid non-const API */
    strncpy(src_copy, src, INET6_ADDRSTRLEN + 1);
    src_copy[INET6_ADDRSTRLEN] = 0;

    if (WSAStringToAddress(src_copy, af, NULL, (struct sockaddr *) &ss, &size) == 0) {
        switch (af) {
            case AF_INET:
                *(struct in_addr *) dst = ((struct sockaddr_in *) &ss)->sin_addr;
                return 1;
            case AF_INET6:
                *(struct in6_addr *) dst = ((struct sockaddr_in6 *) &ss)->sin6_addr;
                return 1;
        }
    }
    return 0;
}


const char *inet_ntop(int af, const void *src, char *dst, socklen_t size) {
    struct sockaddr_storage ss;
    unsigned long s = size;

    ZeroMemory(&ss, sizeof(ss));
    ss.ss_family = af;

    switch (af) {
        case AF_INET:
            ((struct sockaddr_in *) &ss)->sin_addr = *(struct in_addr *) src;
            break;
        case AF_INET6:
            ((struct sockaddr_in6 *) &ss)->sin6_addr = *(struct in6_addr *) src;
            break;
        default:
            return NULL;
    }
    /* cannot directly use &size because of strict aliasing rules */
    return (WSAAddressToString((struct sockaddr *) &ss, sizeof(ss), NULL, dst, &s) == 0) ?
           dst : NULL;
}
#endif
