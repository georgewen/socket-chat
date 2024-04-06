#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>


// below are data structure used
typedef struct Message {
    char* text;
    struct Message* next;
} Message;

typedef struct DictionaryEntry {
    char* key;
    Message* messages;
    struct DictionaryEntry* next;
} DictionaryEntry;

DictionaryEntry* dictionary = NULL;

void insertMessage(char* key, char* message) {
    // Check if the key already exists in the dictionary
    DictionaryEntry* current = dictionary;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            // Create a new message and add it to the list
            Message* newMessage = (Message*)malloc(sizeof(Message));
            newMessage->text = strdup(message);
            newMessage->next = current->messages;
            current->messages = newMessage;
            return;
        }
        current = current->next;
    }
    
    // If the key doesn't exist, create a new entry
    DictionaryEntry* newEntry = (DictionaryEntry*)malloc(sizeof(DictionaryEntry));
    newEntry->key = strdup(key);
    newEntry->messages = NULL;
    newEntry->next = dictionary;
    dictionary = newEntry;
    
    // Create a new message and add it to the list
    Message* newMessage = (Message*)malloc(sizeof(Message));
    newMessage->text = strdup(message);
    newMessage->next = newEntry->messages;
    newEntry->messages = newMessage;
}

char* popMessage(char* key) {
    DictionaryEntry* current = dictionary;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            // Get the first message, store its text, and remove it from the list
            Message* firstMessage = current->messages;
            char* messageText = strdup(firstMessage->text);
            current->messages = firstMessage->next;
            free(firstMessage);
            return messageText;
        }
        current = current->next;
    }
    
    // If the key doesn't exist or there are no messages, return NULL
    return NULL;
}

int countMessages(char* key) {
    DictionaryEntry* current = dictionary;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            int count = 0;
            Message* messagePtr = current->messages;
            while (messagePtr != NULL) {
                count++;
                messagePtr = messagePtr->next;
            }
            return count;
        }
        current = current->next;
    }
    
    // If the key doesn't exist, return 0
    return 0;
}
//end of data structure

#define MAX_CLIENTS 10
#define BUFFER_SIZE 256
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
    int client_socket = *((int *)arg);
    char buffer[BUFFER_SIZE];
    int readSize;

    while (1) {
        readSize = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (readSize == 0){
            printf("exiting...");
            close(client_socket);
            break;
        } else {
            buffer[readSize] = '\0'; // Null-terminate the received message
            printf("message received: %s", buffer);
        }
        memset(buffer, 0, BUFFER_SIZE); // Clear the buffer for the next message
    }
    // // Receive messages from client
    // while ((readSize = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
    //     buffer[readSize] = '\0';
    //     //sendMessageToAllClients(buffer, client_socket);
    //     printf("message received: %s", buffer);
    //     memset(buffer, 0, BUFFER_SIZE);
    // }

    // if (readSize == 0 || readSize == -1) {
    //     close(client_socket);
    //     // Remove client from the array
    //     pthread_mutex_lock(&clients_mutex);
    //     for (int i = 0; i < MAX_CLIENTS; ++i) {
    //         if (clientSockets[i] == client_socket) {
    //             clientSockets[i] = 0;
    //             break;
    //         }
    //     }
    //     pthread_mutex_unlock(&clients_mutex);
    // }

    free(arg);
    pthread_exit(NULL);
}

int main() {
    int serverSocket, *clientSocket;
    struct sockaddr_in server_addr, clientAddr;
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
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind
    if (bind(serverSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }
    printf("Bind done\n");

    // Listen
    listen(serverSocket, 3);

    printf("Waiting for incoming connections...\n");
    int c = sizeof(struct sockaddr_in);
    while ((clientSocket = malloc(sizeof(int)), *clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, (socklen_t*)&c))) {
        printf("Connection accepted\n");

        // Add client socket to array
        pthread_mutex_lock(&clients_mutex);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clientSockets[i] == 0) {
                clientSockets[i] = *clientSocket;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        if (pthread_create(&tid, NULL, handleClient, (void*)clientSocket) < 0) {
            perror("Could not create thread");
            return 1;
        }
    }

    if (clientSocket < 0) {
        perror("Accept failed");
        return 1;
    }

    return 0;
}
