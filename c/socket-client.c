#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888

// Function to handle receiving messages
void *receiveMessage(void *socket) {
    int sock = *((int *)socket);
    char message[2048];
    while(1) {
        int length = recv(sock, message, 2048, 0);
        if (length > 0) {
            message[length] = '\0';
            printf("%s\n", message);
        }
    }
}

int main() {
    struct sockaddr_in server;
    int sock, read_size;
    char message[2048];
    pthread_t recvThread;

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        printf("Could not create socket");
    }
    printf("Socket created\n");
    
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    // Connect to remote server
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("Connect failed. Error");
        return 1;
    }
    
    printf("Connected\n");

    // Create receiving thread
    if(pthread_create(&recvThread, NULL, receiveMessage, (void*)&sock) < 0) {
        perror("could not create thread");
        return 1;
    }

    // Send messages
    while(fgets(message, 2048, stdin) > 0) {
        send(sock, message, strlen(message), 0);
    }

    close(sock);
    return 0;
}
