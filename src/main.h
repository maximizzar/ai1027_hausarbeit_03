//
// Created by maximizzar on 02.07.24.
//

#ifndef MAIN_H
#define MAIN_H

#include <argp.h>

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>

#include <string.h>
#include <unistd.h>

#include <signal.h>

#define BUFFER_SIZE 1024
#define MAX_ADDRESSES 10

#define MAX_CLIENTS 10

/* Program documentation. */
const char *argp_program_version = "SMB 1.0.0";
const char *argp_program_bug_address = "<mail@maximizzar.de>";
static char doc[] = "SMB: Simple Message Broker";

// Enumeration for app types
enum Type {
    BROKER,
    PUBLISHER,
    SUBSCRIBER,
};

/* Struct to represent Deserialized Data from Socket */
struct SocketData {
    enum Type type;
    char topic[BUFFER_SIZE - (BUFFER_SIZE / 4)];
    char data[BUFFER_SIZE - (BUFFER_SIZE / 4)];
};

/* CircularBuffer to store some ClientData */
typedef struct {
    struct SocketData* array;
    int size;
    int index;
} CircularBuffer;

CircularBuffer* createCircularBuffer(int size) {
    CircularBuffer* buffer = (CircularBuffer*)malloc(sizeof(CircularBuffer));
    buffer->array = (struct SocketData*)malloc(size * sizeof(struct SocketData));
    buffer->size = size;
    buffer->index = 0;
    return buffer;
}
void addToCircularBuffer(CircularBuffer* buffer, struct SocketData data) {
    if (data.type == PUBLISHER) {
        buffer->array[buffer->index] = data;
        buffer->index = (buffer->index + 1) % buffer->size;
    }
}
void printCircularBuffer(CircularBuffer* buffer) {
    printf("Circular Buffer Contents: ");
    for (int i = buffer->index; i < buffer->size + buffer->index; i++) {
        int index = i % buffer->size; // Wrap around to the beginning of the buffer
        printf("%s %s ", buffer->array[index].topic, buffer->array[index].data);
    }
    printf("\n");
}

// Structure to store client information
struct Client {
    char ip[INET6_ADDRSTRLEN];
    unsigned short port;
    enum Type type;
    char *topic;
    struct Client* next;
};

// Linked list of all clients
struct Client* client_list = NULL;

/* cli arguments */
struct Arguments {
    bool verbose;
    char *topic, *host;
    enum Type type;
    unsigned short port;
};

#endif
