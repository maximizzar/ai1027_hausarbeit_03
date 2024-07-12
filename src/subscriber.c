//
// Created by maximizzar on 02.07.24.
//

#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "main.h"

_Noreturn int subscriber(struct Arguments arguments) {
    /* Define client_socket, clear the memory and set their defaults */
    struct Socket clientSocket;
    memset(&clientSocket, 0, sizeof clientSocket);
    clientSocket.status = 0;
    clientSocket.client_fd = 0;
    clientSocket.address_count = 0;
    clientSocket.client_address_length = sizeof(clientSocket.client_address);

    clientSocket.hints.ai_family = PF_UNSPEC;
    clientSocket.hints.ai_socktype = SOCKET_TYPE;
    clientSocket.hints.ai_flags = AI_NUMERICSERV;

    if (get_ip_from_fqdn(&clientSocket, arguments.port) < 0) {
        printf("Can't connect to an address successfully.");
        exit(EXIT_FAILURE);
    }

    // exit if no topic
    if (arguments.topic == NULL) {
        printf("A Subscriber needs to have an Topic set.\nPlease use --topic or -t and set one.");
        exit(1);
    }

    // subscribe to topic
    strcpy(clientSocket.buffer, "SUB ");
    strcat(clientSocket.buffer, arguments.topic);
    printf("%s", clientSocket.buffer);

    while (true) {
        while (read(clientSocket.client_fd, clientSocket.buffer, BUFFER_SIZE)) {
            printf("%s", clientSocket.buffer);
            memset(clientSocket.buffer, 0, BUFFER_SIZE);
        }
        perror("Failed to read from socket.");
    }
}
