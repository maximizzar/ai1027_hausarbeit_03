//
// Created by maximizzar on 17.07.24.
//

#ifndef AI1027_HAUSARBEIT_03_SMBCOMMON_H
#define AI1027_HAUSARBEIT_03_SMBCOMMON_H

/* include std*.h */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <string.h>

#include <sys/socket.h>
#include <sys/random.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <time.h>

#define SERVER_IP "127.0.0.1"
#define PORT 64002
#define MAX_BUFFER_SIZE 1024

typedef struct {
    time_t unix_timestamp;
    char topic[MAX_BUFFER_SIZE / 2 - 8];
    char data[MAX_BUFFER_SIZE / 2 - 8];
} Message;

typedef struct {
    Message *message;
    int size;
    int index;
} CircularBuffer;

CircularBuffer* create_circular_buffer(int size);
void add_to_circular_buffer(CircularBuffer* circular_buffer, Message* message);

bool match_topic(char *a, char *b);

/*
 * Serialization and Deserialization
 */

int socket_serialization(char buffer[MAX_BUFFER_SIZE], Message *message);
int socket_deserialization(char buffer[MAX_BUFFER_SIZE], Message *message);

char* timestamp_to_string(time_t timestamp);

#endif //AI1027_HAUSARBEIT_03_SMBCOMMON_H
