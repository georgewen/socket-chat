// Author: George Wen
// A C implementation for a multi-client socket chat client.

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

// Thread function for receiving messages from the server
void *receiveMessage(void *socket) {
    int sock = *((int *)socket);
    char message[256];
    while(1) {
        // Attempt to read a message from the socket
        int length = read(sock, message, 256);
        // If message length is positive, print the message
        if (length > 0) {
            message[length] = '\0'; // Null-terminate the message
            printf("%s\n", message);
        }
    }
}

int main(int argc, char *argv[]) {
    // Check for correct number of command-line arguments
    if (argc != 3) {
        printf("Usage: %s <server IP> <port>\n", argv[0]);
        return 1;
    }
    char* host = argv[1]; // Server IP address
    int port = atoi(argv[2]); // Server port number

    struct sockaddr_in server_addr;
    int sockfd;
    char username[50];
    char buffer[256];
    pthread_t recvThread;

    // Create a TCP socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Could not create socket \n");
    }
    printf("Socket created\n");
    
    // Set server address and port
    server_addr.sin_addr.s_addr = inet_addr(host);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connect failed. Error");
        return 1;
    }
    printf("Connected\n");

    // Get username from the user, send login message to the server
    printf("Please enter your username: ");
    fgets(username, 50, stdin);
    username[strcspn(username, "\n")] = '\0'; // Remove newline
    sprintf(buffer, "LOGIN %s", username);
    send(sockfd, buffer, strlen(buffer), 0);

    printf("Logged in\n");

    // Create a thread for receiving messages from the server
    if(pthread_create(&recvThread, NULL, receiveMessage, (void*)&sockfd) < 0) {
        perror("Could not create thread");
        return 1;
    }

    int sending = 0; // Flag indicating the next message to send is part of a composition
    // Main loop for sending messages
    while(fgets(buffer, BUFFER_SIZE, stdin) > 0) {
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline

        if (sending) {
            // If currently composing a message, send the message text
            send(sockfd, buffer, strlen(buffer), 0);
            sending = 0;
        } else if (strncmp(buffer, "COMPOSE", 7) == 0) {
            // If starting a new message composition
            char* username = strchr(buffer, ' ');
            if (username == NULL || strlen(username + 1) == 0) {
                printf("Error: correct message format is COMPOSE <username>\n");
            } else {
                send(sockfd, buffer, strlen(buffer), 0);
                sending = 1; // Next input will be the message body
            }
        } else if (strncmp(buffer, "READ", 4) == 0 || strncmp(buffer, "EXIT", 4) == 0) {
            // For READ and EXIT commands, send them directly
            send(sockfd, buffer, strlen(buffer), 0);
            if (strncmp(buffer, "EXIT", 4) == 0) {
                printf("Client Exiting\n");
                break; // Exit the loop on EXIT command
            }
        } else {
            printf("Error: unknown command\n");
        }
    }

    printf("Closing down\n");
    close(sockfd); // Close the socket
    // Optionally wait for the receiving thread to finish
    // pthread_join(recvThread, NULL);
    // pthread_exit(NULL); // Properly exit the thread
    return 0;
}
