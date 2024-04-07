#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void insert(char* key, char* message) {
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

char* readAndRemoveFirstMessage(char* key) {
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

void printDictionary() {
    DictionaryEntry* current = dictionary;
    while (current != NULL) {
        printf("%s:\n", current->key);
        Message* messagePtr = current->messages;
        while (messagePtr != NULL) {
            printf("- %s\n", messagePtr->text);
            messagePtr = messagePtr->next;
        }
        printf("\n");
        current = current->next;
    }
}

int main() {
    insert("user1", "Hello, world!");
    insert("user1", "This is a second message.");
    insert("user2", "Hey there!");
    insert("user2", "How's it going?");
    insert("user3", "Just checking in.");

    int messageCount = countMessages("user1");
    printf("Number of messages for user1: %d\n", messageCount);

    char* firstMessage = readAndRemoveFirstMessage("user1");
    printf("First message for user1: %s\n", firstMessage);
    free(firstMessage);
    
     messageCount = countMessages("user1");
    printf("Number of messages for user1: %d\n", messageCount);
    
    printDictionary();
    
    return 0;
}