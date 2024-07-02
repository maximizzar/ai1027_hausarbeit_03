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
static char doc[] = "SMB Publisher: Simple Message Broker";

/* options */
static struct argp_option options[] = {
        {"role",  'r', "ROLE",  0, "BROKER, PUB, SUB" },
        {"topic", 't', "TOPIC", 0, "Topic to write to"},
        {"port",  'p', "PORT",  0, "Server Port" },
        {"host",  'h', "HOST",  0, "Host"},
        { 0 }
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
    arguments.host = "::1";
    arguments.port = 8080;
    argp_parse (&argp, argc, argv, 0, 0, &arguments);

    if (strcmp(arguments.role, "BROKER") == 0) broker(arguments);
    else if (strcmp(arguments.role, "PUB") == 0) publisher(arguments);
    else if (strcmp(arguments.role, "SUB") == 0) subscriber(arguments);
    else {
        printf("unknown role %s! Please define a valid role.", arguments.role);
    }
    return EXIT_SUCCESS;
}
