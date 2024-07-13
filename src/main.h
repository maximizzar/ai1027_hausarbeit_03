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

/* cli arguments */
struct Arguments {
    bool verbose;
    char *role, *topic, *host;
    unsigned short port;
};

#endif
