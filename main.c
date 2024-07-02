//
// Created by maximizzar on 02.07.24.
//

#include <argp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "src/broker.c"
#include "src/publisher.c"
#include "src/subscriber.c"

#define SOCKET_TYPE SOCK_DGRAM
#define BUFFER_SIZE 4096

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

/* cli arguments */
struct arguments {
    char *role, *topic, *host;
    unsigned short port;
};

/* option parsing */
static error_t parse_option(int key, char *arg, struct argp_state *state) {
    struct arguments *arguments = state->input;

    switch (key) {
        case 'r':
            if (strcmp(arg, "BROKER") == 0 || strcmp(arg, "PUB") == 0 || strcmp(arg, "SUB") == 0) {
                arguments->role;
                break;
            }
            printf("ERROR: Unknown role %s", arg);
            return EXIT_FAILURE;
        case 't': arguments->topic = arg; break;
        case 'p': arguments->port = strtol(arg, NULL, 0); break;
        case 'h': arguments->host = arg; break;
        default: return ARGP_ERR_UNKNOWN;


    }
    if (strcmp(arguments->role, "") == 0) {
        argp_failure(state, 1, 0, "you need to provide an role");
        exit(ARGP_ERR_UNKNOWN);
    }
    return EXIT_SUCCESS;
}

/* publisher argp parser. */
static struct argp argp = { options, parse_option, NULL, doc };

int main(int argc, char *argv[]) {
    /* Set arg defaults and then parse cli-arg into struct */
    struct arguments arguments;
    arguments.host = "::1";
    arguments.port = 8080;
    argp_parse (&argp, argc, argv, 0, 0, &arguments);

    if (strcmp(arguments.role, "BROKER") == 0) broker(arguments);
    else if (strcmp(arguments.role, "PUB") == 0) publisher(arguments);
    else subscriber(arguments);
    return EXIT_SUCCESS;
}
