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

#include <time.h>

#define SERVER_IP "127.0.0.1"
#define PORT 64002
#define MAX_CLIENTS 5
#define MAX_BUFFER_SIZE 2048

typedef enum {
    PUBLISHER,
    SUBSCRIBER,
} ClientType;

typedef struct {
    time_t unix_timestamp;
    char topic[MAX_BUFFER_SIZE / 2 - 8];
    char data[MAX_BUFFER_SIZE / 2 - 8];
} Message;

int socket_serialization(char buffer[MAX_BUFFER_SIZE], Message *message);
int socket_deserialization(char buffer[MAX_BUFFER_SIZE], Message *message);

char* timestamp_to_string(time_t timestamp);
#endif //AI1027_HAUSARBEIT_03_SMBCOMMON_H

