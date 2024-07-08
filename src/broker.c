//
// Created by maximizzar on 02.07.24.
//

#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdlib.h>

#include <stdio.h>
#include <string.h>
#include "main.h"

_Noreturn int broker(struct Arguments arguments) {
    /* Define socket */
    struct Socket broker_socket;
    broker_socket.broker_fd = 0;
    broker_socket.client_fd = 0;
    broker_socket.opt = 0;

    // fill broker_address struct
    memset((void *) &broker_socket.broker_address, 0, sizeof(broker_socket.broker_address));
    broker_socket.broker_address.sin6_addr = in6addr_any;
    broker_socket.broker_address.sin6_flowinfo = 0;
    broker_socket.broker_address.sin6_port = htons(arguments.port);
    broker_socket.broker_address.sin6_family = PF_INET6;

    // use setsockopt to get ipv4 working over ipv6 socket
    if (setsockopt(broker_socket.broker_fd, IPPROTO_IPV6, IPV6_V6ONLY, &broker_socket.opt, sizeof(broker_socket.opt)) < 0) {
        perror("Error setting socket option");
        exit(EXIT_FAILURE);
    }

    // try to bind, exit when failure
    if (bind(broker_socket.broker_fd, (struct sockaddr *) &broker_socket.broker_address, sizeof(broker_socket.broker_address)) < 0) {
        perror("Bind failed.");
        exit(EXIT_FAILURE);
    }

    // try to listen, exit when failure
    if (listen(broker_socket.broker_fd, 5) < 0) {
        perror("Listen failed.");
        exit(EXIT_FAILURE);
    }

    while (true) {
        broker_socket.client_address_length = sizeof(broker_socket.client_address);
        if ((broker_socket.client_fd = accept(broker_socket.broker_fd, (struct sockaddr *) &broker_socket.client_address, &broker_socket.client_address_length)) < 0) {
            perror("Accept failed");
            continue;
        }

        // Handle the client connection

    }
}
