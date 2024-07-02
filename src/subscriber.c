//
// Created by maximizzar on 02.07.24.
//

#include <arpa/inet.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

struct arguments {
    char *role, *topic, *host;
    unsigned short port;
};

int subscriber(struct arguments arguments) {

    return EXIT_SUCCESS;
}