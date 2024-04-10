#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>

// Author: George Wen
// A C implementation for a multi-client socket chat program.
// This server manages messages between clients, allowing users to log in, compose messages, and read messages.

// Definition of a message structure in a linked list format.
typedef struct Message {
    char* from;           // Sender username
    char* text;           // Message text
    struct Message* next; // Pointer to the next message in the list
} Message;

// Dictionary entry structure, acting as a key-value pair where the key is the username.
typedef struct DictionaryEntry {
    char* key;                // Username as the key
    Message* messages;        // Pointer to the first message in the list for this user
    struct DictionaryEntry* next; // Pointer to the next dictionary entry in the list
} DictionaryEntry;

DictionaryEntry* dictionary = NULL; // Global dictionary to store messages for each user

// Function to insert a message for a specific user
void insertMessage(char* key, char* from, char* text) {
    // Iterate through the dictionary to find an entry for the key
    DictionaryEntry* current = dictionary;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            // If the key exists, prepend a new message to the list
            Message* newMessage = (Message*)malloc(sizeof(Message));
            newMessage->from = strdup(from);
            newMessage->text = strdup(text);
            newMessage->next = current->messages;
            current->messages = newMessage;
            return;
        }
        current = current->next;
    }
    
    // If the key doesn't exist, create a new dictionary entry and prepend it
    DictionaryEntry* newEntry = (DictionaryEntry*)malloc(sizeof(DictionaryEntry));
    newEntry->key = strdup(key);
    newEntry->messages = NULL; // Initially, no messages
    newEntry->next = dictionary;
    dictionary = newEntry;
    
    // Add the new message to the newly created entry
    Message* newMessage = (Message*)malloc(sizeof(Message));
    newMessage->from = strdup(from);
    newMessage->text = strdup(text);
    newMessage->next = newEntry->messages;
    newEntry->messages = newMessage;
}

// Function to pop (retrieve and remove) the first message for a specific user
Message* popMessage(char* key) {
    DictionaryEntry* current = dictionary;
    while (current != NULL) {
        if (strcmp(current->key, key) == 0) {
            // If the user has messages, detach the first message from the list and return it
            Message* firstMessage = current->messages;
            if (firstMessage != NULL) {
                current->messages = firstMessage->next;
                return firstMessage;
            }
            break;
        }
        current = current->next;
    }
    // If no messages or the key doesn't exist, return NULL
    return NULL;
}

// Function to count the number of messages for a specific user
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
            return count; // Return the number of messages
        }
        current = current->next;
    }
    // If the key doesn't exist, return 0
    return 0;
}

#define BUFFER_SIZE 256 // Buffer size for reading messages
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER; // Mutex for thread-safe operations on the dictionary

// Thread function to handle communication with a client
void *handleClient(void *arg) {
    int client_socket = *((int *)arg);
    char buffer[BUFFER_SIZE];
    int readSize;

    char currentUser[50] = {0}; // Store the current user's name
    int receiving = 0;          // Flag to indicate if the server is expecting a message
    char toUser[50] = {0};      // Store the recipient user for messages

    while (1) {
        memset(buffer, 0, BUFFER_SIZE); // Clear the buffer
        readSize = recv(client_socket, buffer, BUFFER_SIZE, 0); // Read message from client

        if (readSize <= 0) {
            printf("exiting...\n");
            close(client_socket);
            break;
        }

        printf("Message received: %s\n", buffer);

        // Process commands
        if (strlen(currentUser) == 0) { // If the user has not logged in
            if (strncmp(buffer, "LOGIN", 5) == 0) {
                sscanf(buffer, "LOGIN %s", currentUser);
                int messageCount = countMessages(currentUser);
                char response[256];
                sprintf(response, "%d Messages\n", messageCount);
                send(client_socket, response, strlen(response), 0);
            } else {
                printf("Please issue LOGIN command first\n");
            }
        } else {
            if (receiving) {
                // Store the received message
                char* response = "MESSAGE SENT\n";
                pthread_mutex_lock(&clients_mutex);
                insertMessage(toUser, currentUser, buffer);
                pthread_mutex_unlock(&clients_mutex);
                send(client_socket, response, strlen(response), 0);
                receiving = 0;
            } else {
                // Handle READ, COMPOSE, or EXIT commands
                if (strncmp(buffer, "READ", 4) == 0) {
                    pthread_mutex_lock(&clients_mutex);
                    Message* msg = popMessage(currentUser);
                    pthread_mutex_unlock(&clients_mutex);
                    if (msg != NULL) {
                        send(client_socket, msg->from, strlen(msg->from), 0);
                        send(client_socket, "\n", 1, 0);
                        send(client_socket, msg->text, strlen(msg->text), 0);
                        send(client_socket, "\n", 1, 0);
                        free(msg->from); // Free the memory allocated for the message
                        free(msg->text);
                        free(msg);
                    } else {
                        char* response = "READ ERROR\n";
                        send(client_socket, response, strlen(response), 0);
                    }
                } else if (strncmp(buffer, "COMPOSE", 7) == 0) {
                    sscanf(buffer, "COMPOSE %s", toUser);
                    receiving = 1; // Next message will be stored as a message content
                } else if (strncmp(buffer, "EXIT", 4) == 0) {
                    break; // Exit the loop and close the socket
                } else {
                    char* response = "INVALID COMMAND\n";
                    send(client_socket, response, strlen(response), 0); // Inform the client of the invalid command
                }
            }
        }
    }

    printf("Exiting thread...\n");
    close(client_socket); // Close the client socket on exit
    // pthread_exit(NULL); // Optionally, explicitly exit the thread
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    int port = atoi(argv[1]); // Convert port argument from string to integer

    int serverSocket;
    struct sockaddr_in server_addr, clientAddr;
    pthread_t tid;

    // Create socket
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Could not create socket");
        return 1;
    }
    printf("Socket created\n");

    // Prepare the sockaddr_in structure
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Accept connections on any IP address
    server_addr.sin_port = htons(port);       // Host to network short conversion for port

    // Bind
    if (bind(serverSocket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        return 1;
    }
    printf("Bind done\n");

    // Listen for incoming connections
    listen(serverSocket, 3);
    printf("Waiting for incoming connections...\n");

    int c = sizeof(struct sockaddr_in);
    while (1) {
        int *clientSocket = malloc(sizeof(int));
        *clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, (socklen_t*)&c);
        if (*clientSocket < 0) {
            perror("Accept failed");
            return 1;
        }
        printf("Connection accepted\n");

        // Create a new thread for each client
        if (pthread_create(&tid, NULL, handleClient, (void*)clientSocket) < 0) {
            perror("Could not create thread");
            return 1;
        }
    }

    return 0; // End of program
}
