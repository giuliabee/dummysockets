#include <sys/types.h>  /* for constants (e.g. AF_INET) */
#include <sys/socket.h>        /* for socket(), send(), connect(), recv(), ... */

#include <arpa/inet.h>        /* for inet_pton(), htons() */
#include <stdio.h>        /* for snprintf(), printf() */
#include <string.h>        /* for memset(), strlen() */

#include <openssl/md5.h>

#define EXIT_SUCCESS 0
#define EXIT_ERROR   1

#define SERVER_ADDRESS "127.0.0.1"
#define SERVER_PORT    12345

#define BUFFER_SIZE 256
#define FILE_SIZE 2048

struct MD5Context {
    int buf[4];
    int bits[2];
    char in[64];
};

int main() {
        int sock, err, bytes, len;
        char c;
        char msg[BUFFER_SIZE];
        char path[BUFFER_SIZE];
        char file[FILE_SIZE];
        char digest[16];
        
        FILE *fp;
        
        struct sockaddr_in dest_addr;
        
        struct MD5Context context;

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

        printf("Enter name of the file to send: \n");
        fgets(path, BUFFER_SIZE, stdin);
        
        char *p_chr=strchr(path, '\n');
        if(p_chr != NULL) *p_chr='\0';
        
        fp = fopen(path, "r");
        
        while (fp == NULL) {
                printf("File does not exist! Enter name of the file to send: \n");
                fgets(path, BUFFER_SIZE, stdin);
                
                char *p_chr=strchr(path, '\n');
                if(p_chr != NULL) *p_chr='\0';
                
                fp = fopen(path, "r");
        }
        
        err = fgets(file, FILE_SIZE, fp);
                
        if (err <= 0) {
                perror("Failed to read file\n");
                return EXIT_ERROR; 
        }
        
        printf("Read file %s \n", path);
        
        MD5Init(&context);
        MD5Update(&context, file, FILE_SIZE);
        MD5Final(digest, &context);
        
        len = strlen(digest);

        bytes = sendto(sock, &digest, len, 0, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in));
        
        if (bytes < 0) {
                perror("Failed to send md5sum"); 
                return EXIT_ERROR; 
        }

        len = strlen(file);
        
        bytes = sendto(sock, &file, len, 0, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr_in));
        
        if (bytes < 0) {
                perror("Failed to send file"); 
                return EXIT_ERROR; 
        }

        printf("Sent %d bytes\n", bytes);
        
        fclose (fp);

        bytes = recv(sock, msg, BUFFER_SIZE, 0);

        if (bytes < 0) {
                perror("Failed to receive"); 
                return EXIT_ERROR; 
        }

        printf("Received %d bytes: %s", bytes, msg);

        return EXIT_SUCCESS;
}

