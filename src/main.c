//
// Created by maximizzar on 02.07.24.
//

#include <argp.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "main.h"
#include "broker.h"
#include "publisher.h"
#include "subscriber.h"

const char *argp_program_version = "SMB 1.0.0";
const char *argp_program_bug_address = "<mail@maximizzar.de>";

/* Program documentation. */
static char doc[] = "SMB: Simple Message Broker";

/* options */
static struct argp_option options[] = {
        {"role",  'r', "ROLE",  0, "BROKER, PUB, SUB" },
        {"topic", 't', "TOPIC", 0, "Topic to write to"},
        {"port",  'p', "PORT",  0, "Server Port" },
        {"host",  'h', "HOST",  0, "Host"},
        { 0 },
};

/* option parsing */
static error_t parse_option(int key, char *arg, struct argp_state *state) {
    struct Arguments *arguments = state->input;

    switch (key) {
        case 'r':
            if (strcmp(arg, "BROKER") == 0 || strcmp(arg, "PUB") == 0 || strcmp(arg, "SUB") == 0) {
                arguments->role = arg;
                break;
            }
            printf("ERROR: Unknown role %s", arg);
            return EXIT_FAILURE;
        case 't': arguments->topic = arg; break;
        case 'p': arguments->port = strtol(arg, NULL, 0); break;
        case 'h': arguments->host = arg; break;
        default: return ARGP_ERR_UNKNOWN;
    }
    return EXIT_SUCCESS;
}

/* publisher argp parser. */
static struct argp argp = { options, parse_option, NULL, doc };

int main(int argc, char *argv[]) {
    /* Set arg defaults and then parse cli-arg into struct */
    struct Arguments arguments;
    memset(&arguments, 0, sizeof arguments);
    arguments.host = "::1";
    arguments.port = 8080;
    argp_parse (&argp, argc, argv, 0, 0, &arguments);

    if (arguments.role == NULL) {
        printf("You need to set an role.");
        exit(1);
    }

    if (strcmp(arguments.role, "BROKER") == 0) broker(arguments);
    else if (strcmp(arguments.role, "PUB") == 0) publisher(arguments);
    else if (strcmp(arguments.role, "SUB") == 0) subscriber(arguments);
    else {
        printf("unknown role %s! Please define a valid role.", arguments.role);
    }
    return EXIT_SUCCESS;
}

int get_ip_from_fqdn(struct Socket *clientSocket, unsigned short port) {
    /*
     * Loop over addresses in clientSocket.address_list provided by getaddrinfo
     * - if an address works the loop breaks with a successful connection
     * - if all addresses don't work, the client closes
     */
    for (int i = 0; i < clientSocket->address_count; i++) {
        char ip_str[INET6_ADDRSTRLEN];
        memset(&clientSocket->client_address, 0, sizeof clientSocket->client_address);
        clientSocket->client_address.sin6_port = htons(port);

        /* address in clientSocket.address_list is a v4 address and needs a v4 socket */
        if (clientSocket->address_list[i].ss_family == PF_INET) {
            clientSocket->client_address.sin6_family = PF_INET;
            // Create v4 Socket
            if ((clientSocket->client_fd = socket(PF_INET, SOCKET_TYPE, 0)) < 0) {
                perror("socket creation failed");
                exit(EXIT_FAILURE);
            }
        }
            /* address in clientSocket.address_list is a v6 address and needs a v6 socket */
        else {
            clientSocket->client_address.sin6_family = PF_INET6;
            // Create v6 Socket
            if ((clientSocket->client_fd = socket(PF_INET6, SOCKET_TYPE, 0)) < 0) {
                perror("socket creation failed");
                exit(EXIT_FAILURE);
            }
        }

        if ((clientSocket->status = connect(clientSocket->client_fd, (struct sockaddr *) &clientSocket->address_list[i], sizeof(clientSocket->address_list[i]))) < 0) {
            perror("Connection Failed");
            if (i + 1 == clientSocket->address_count) {
                exit(EXIT_FAILURE);
            }
            continue;
        }
        break;
    }
    return EXIT_SUCCESS;
}
