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

#define BUFFER_SIZE 1024
#define MAX_ADDRESSES 10

#define MAX_CLIENTS 10
#define MAX_TOPIC_LENGTH 50

// Enumeration for Roles
enum Role {
    BROKER,
    PUBLISHER,
    SUBSCRIBER,
};

// Structure to store client information
struct Client {
    char ip[INET6_ADDRSTRLEN];
    unsigned short port;
    enum Role type;
    struct Client* next;
};

// Linked list of all clients
struct Client* client_list = NULL;

/* cli arguments */
struct Arguments {
    bool verbose;
    char *topic, *host;
    enum Role role;
    unsigned short port;
};

#endif
