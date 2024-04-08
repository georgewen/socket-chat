#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>


// below are data structure used
typedef struct Message {
    char* from;
    char* text;
    struct Message* next;
} Message;

typedef struct DictionaryEntry {
    char* key;
    Message* messages;
    struct DictionaryEntry* next;
} DictionaryEntry;

DictionaryEntry* dictionary = NULL;

void insertMessage(char* key, char* from, char* text) {
    // Check if the key already exists in the dictionary
    DictionaryEntry* current = dictionary;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            // Create a new message and add it to the list
            Message* newMessage = (Message*)malloc(sizeof(Message));
            newMessage->from = strdup(from);
            newMessage->text = strdup(text);
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
    newMessage->from = strdup(from);
    newMessage->text = strdup(text);
    newMessage->next = newEntry->messages;
    newEntry->messages = newMessage;
}

Message* popMessage(char* key) {
    DictionaryEntry* current = dictionary;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            // Get the first message, store its text, and remove it from the list
            Message* firstMessage = current->messages;
            // char* messageText = strdup(firstMessage->text);
            current->messages = firstMessage->next;
            // free(firstMessage);
            // return messageText;
            return firstMessage;
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

    char currentUser[50] = {0}; // Store the current user
    int receiving = 0; // Flag to indicate if the server is receiving a message to store
    char toUser[50] = {0}; // Store the recipient user for messages

    while (1) {

        memset(buffer, 0, BUFFER_SIZE); // Clear the buffer for the next message
        readSize = recv(client_socket, buffer, BUFFER_SIZE, 0);

        if (readSize <= 0){
            printf("exiting...");
            close(client_socket);
            break;
        } 

        printf("message received: %s \n", buffer);

        // Process commands
        if (strlen(currentUser) == 0) { // If not logged in
            if (strncmp(buffer, "LOGIN", 5) == 0) {
                sscanf(buffer, "LOGIN %s", currentUser);
                int messageCount = countMessages(currentUser);

                char response[256];
                sprintf(response, "%d Messages\n", messageCount); // Placeholder for actual message count
                send(client_socket, response, strlen(response), 0);
            } else {
                printf("Please issue LOGIN command first\n");
            }
        } else {
            if (receiving) {
                // Here you would store the received message for the user `toUser`
                char* response = "MESSAGE SENT\n";
                insertMessage(toUser, currentUser, buffer);
                send(client_socket, response, strlen(response), 0);
                receiving = 0;
            } else {

                if (strncmp(buffer, "READ", 4) == 0) {
                    // Here you would pop and send the last message for currentUser
                    // Placeholder for message reading logic
                    int messageCount = countMessages(currentUser);
                    char* response;
                    if (messageCount > 0) {
                        Message* msg =  popMessage(currentUser);
                        char* from = msg->from ;
                        char* text = msg->text ;
                        strcat(from, "\n");
                        strcat(text, "\n");
                        send(client_socket, from, strlen(from), 0);
                        send(client_socket, text, strlen(text), 0);
                    }
                    else
                    {
                        response = "READ ERROR";
                        send(client_socket, response, strlen(response), 0);
                    }
                } else if (strncmp(buffer, "COMPOSE", 7) == 0) {
                    //printf("receiving... \n");
                    sscanf(buffer, "COMPOSE %s", toUser);
                    receiving = 1;
                } else if (strncmp(buffer, "EXIT", 4) == 0) {
                    //printf("Exiting...\n");
                    //send(client_socket, exitMsg, strlen(exitMsg), 0);
                    break;
                } else {
                    char* unknownCmdMsg = "Unknown command\n";
                    //send(client_socket, unknownCmdMsg, strlen(unknownCmdMsg), 0);
                    break;
                }
            }
        }

    }

    printf("exiting thread...");
    close(client_socket);
    //free(arg);
    //pthread_exit(NULL);
}

  
int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }
    int port = atoi(argv[1]);


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
    server_addr.sin_port = htons(port);

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
