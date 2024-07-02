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

int subscriber(struct Arguments arguments) {
    printf("%s!!!", arguments.role);
    return EXIT_SUCCESS;
}
