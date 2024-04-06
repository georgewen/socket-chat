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

int main() {
    struct sockaddr_in server_addr;
    int sockfd, read_size;
    char username[50];
    char message[256];
    char buffer[256];
    pthread_t recvThread;

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Could not create socket");
    }
    printf("Socket created\n");
    
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

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

    // Send messages
    // while(fgets(message, 2048, stdin) > 0) {
    //     send(sockfd, message, strlen(message), 0);
    // }


    int sending = 0;
    while (1) {
        //printf("> ");
        bzero(message,256);
        fgets(message, 255, stdin);
        //printf("msg: %zu \n", strlen(message));
        //printf("char: %d %c \n",message[0], message[0]);

        if (message[0] == '\0') continue;
        //printf("message 1: %s \n", message);
        message[strcspn(message, "\n")] = '\0';

        if (sending) {
            printf("sending message \n");
            send(sockfd, message, strlen(message), 0);
            sending = 0;
        } else if (strncmp(message, "COMPOSE", 7) == 0) {
            //printf("message 2: %s", message);
            char* username = strchr(message, ' ');
            //printf("username: %s", username);
            if (username == NULL || strlen(username + 1) == 0) {
                printf("error: correct message format is COMPOSE <username>\n");
            } else {
                sending = 1;
                printf("set sending = 1 \n");
            }
        } else if (strncmp(message, "READ", 4) == 0) {
            // Implement READ functionality
        } else if (strcmp(message, "EXIT") == 0) {
            printf("Client Exiting \n");
            send(sockfd, message, strlen(message), 0);
            break;
        } else {
            printf("error command\n");
        }

    }    
    printf("closing down");
    close(sockfd);
    pthread_join(recvThread, NULL);
    return 0;
}
