#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 2048
#define PORT 8888

int clientSockets[MAX_CLIENTS];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function to send message to all clients except the sender
void sendMessageToAllClients(char *msg, int sender) {
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        if (clientSockets[i] != 0 && clientSockets[i] != sender) {
            if (write(clientSockets[i], msg, strlen(msg)) < 0) {
                perror("Failed to send message");
                break;
            }
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Thread function to handle communication with a client
void *handleClient(void *arg) {
    int sock = *((int *)arg);
    char buffer[BUFFER_SIZE];
    int readSize;

    // Receive messages from client
    while ((readSize = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        buffer[readSize] = '\0';
        sendMessageToAllClients(buffer, sock);
        memset(buffer, 0, BUFFER_SIZE);
    }

    if (readSize == 0 || readSize == -1) {
        close(sock);
        // Remove client from the array
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; ++i) {
            if (clientSockets[i] == sock) {
                clientSockets[i] = 0;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);
    }

    free(arg);
    pthread_exit(NULL);
}

int main() {
    int serverSocket, *newSock;
    struct sockaddr_in serverAddr, clientAddr;
    pthread_t tid;

    // Initialize all client sockets to 0
    for (int i = 0; i < MAX_CLIENTS; ++i) {
        clientSockets[i] = 0;
    }

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Could not create socket");
        return 1;
    }
    printf("Socket created\n");

    // Prepare the sockaddr_in structure
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    // Bind
    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Bind failed");
        return 1;
    }
    printf("Bind done\n");

    // Listen
    listen(serverSocket, 3);

    printf("Waiting for incoming connections...\n");
    int c = sizeof(struct sockaddr_in);
    while ((newSock = malloc(sizeof(int)), *newSock = accept(serverSocket, (struct sockaddr *)&clientAddr, (socklen_t*)&c))) {
        printf("Connection accepted\n");

        // Add client socket to array
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clientSockets[i] == 0) {
                clientSockets[i] = *newSock;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        if (pthread_create(&tid, NULL, handleClient, (void*)newSock) < 0) {
            perror("Could not create thread");
            return 1;
        }
    }

    if (newSock < 0) {
        perror("Accept failed");
        return 1;
    }

    return 0;
}
