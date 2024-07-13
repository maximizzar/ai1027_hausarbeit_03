//
// Created by maximizzar on 13.07.24.
//

#ifndef SERVER_H
#define SERVER_H

#define MAX_CLIENTS 10
#define MAX_TOPIC_LENGTH 50

// Enumeration for client types
enum ClientType {
    PUBLISHER,
    SUBSCRIBER
};

// Structure to store client information
struct Client {
    char ip[INET_ADDRSTRLEN];
    unsigned short port;
    enum ClientType type;
    struct Client* next;
};

// Linked list of all clients
struct Client* client_list = NULL;
#endif
