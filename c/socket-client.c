#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define PORT 8888
#define BUFFER_SIZE 255

// Function to handle receiving messages
void *receiveMessage(void *socket) {
    int sock = *((int *)socket);
    char message[256];
    while(1) {
        //int length = recv(sock, message, 2048, 0);
        int length = read(sock, message, 256);
        if (length > 0) {
            message[length] = '\0';
            printf("%s\n", message);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server IP> <port>\n", argv[0]);
        return 1;
    }
    char* host = argv[1];
    int port = atoi(argv[2]);

    struct sockaddr_in server_addr;
    int sockfd, read_size;
    char username[50];
    char message[256];
    char buffer[256];
    pthread_t recvThread;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Could not create socket \n");
    }
    printf("Socket created\n");
    
    server_addr.sin_addr.s_addr = inet_addr(host);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // Connect to remote server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed. Error");
        return 1;
    }
    
    printf("Connected\n");

    printf("Please enter your username: ");
    //scanf("%s", username);
    fgets(username, 50, stdin);
    sprintf(buffer, "LOGIN %s", username);
    send(sockfd, buffer, strlen(buffer), 0);

    printf("logged in\n");

    // Create receiving thread
    if(pthread_create(&recvThread, NULL, receiveMessage, (void*)&sockfd) < 0) {
        perror("could not create thread");
        return 1;
    }

    int sending = 0;
    //Send messages
    while(fgets(message, 255, stdin) > 0) {

        if (message[0] == '\0') continue;
        message[strcspn(message, "\n")] = '\0';
        if (sending) {
            send(sockfd, message, strlen(message), 0);
            sending = 0;
        } else if (strncmp(message, "COMPOSE", 7) == 0) {
            char* username = strchr(message, ' ');
            if (username == NULL || strlen(username + 1) == 0) {
                printf("error: correct message format is COMPOSE <username>\n");
            } else {
                send(sockfd, message, strlen(message), 0);
                sending = 1;
            }
        } else if (strncmp(message, "READ", 4) == 0) {
            send(sockfd, message, strlen(message), 0);
        } else if (strncmp(message, "EXIT", 4) == 0) {
            printf("Client Exiting \n");
            send(sockfd, message, strlen(message), 0);
            break;
        } else {
            printf("error command\n");
        }

    }
 
    printf("closing down\n");
    close(sockfd);
    //pthread_join(recvThread, NULL);
    //pthread_exit(NULL);
    return 0;
}
