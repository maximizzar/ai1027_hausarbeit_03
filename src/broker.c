//
// Created by maximizzar on 02.07.24.
//

#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdlib.h>

#include <stdio.h>
#include "main.h"

int broker(struct Arguments arguments) {
    /* Define socket */
    struct Socket broker_socket;
    broker_socket.broker_fd = 0;
    broker_socket.client_fd = 0;
    broker_socket.opt = 0;

    return EXIT_SUCCESS;
}
