#include <sys/types.h>  /* for constants (e.g. AF_INET) */
#include <sys/socket.h>        /* for socket(), send(), connect(), recv(), ... */

#include <arpa/inet.h>        /* for inet_pton(), htons() */
#include <stdio.h>        /* for snprintf(), printf() */
#include <string.h>        /* for memset(), strlen() */

#define EXIT_SUCCESS 0
#define EXIT_ERROR   1

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT    12345

#define BUFFER_SIZE 256
#define FILE_SIZE 2048

int main() {
    int sock, err, bytes, len;
    char c;
    char msg[BUFFER_SIZE];
    char path[BUFFER_SIZE];
    char file[FILE_SIZE];

    FILE *fp;

    struct sockaddr_in dest_addr;

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

    printf("Enter name of the file to send: \n");
    fgets(path, BUFFER_SIZE, stdin);

    char *p_chr = strchr(path, '\n');
    if (p_chr != NULL) *p_chr = '\0';

    fp = fopen(path, "r");

    while (fp == NULL) {
        printf("File does not exist! Enter name of the file to send: \n");
        fgets(path, BUFFER_SIZE, stdin);

        char *p_chr = strchr(path, '\n');
        if (p_chr != NULL) *p_chr = '\0';

        fp = fopen(path, "r");
    }

    printf("Read file %s \n", path);

    err = fgets(file, FILE_SIZE, fp);

    if (err <= 0) {
        perror("Failed to read file\n");
        return EXIT_ERROR;
    }

    len = strlen(file);

    bytes = send(sock, &file, len, 0);

    if (bytes < 0) {
        perror("Failed to send");
        return EXIT_ERROR;
    }

    printf("Sent %d bytes\n", bytes);

    fclose(fp);

    bytes = recv(sock, msg, BUFFER_SIZE, 0);

    if (bytes < 0) {
        perror("Failed to receive");
        return EXIT_ERROR;
    }

    printf("Received %d bytes: %s", bytes, msg);

    return EXIT_SUCCESS;
}

