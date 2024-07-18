//
// Created by maximizzar on 17.07.24.
//

#include "smbpublish.h"
#include "smbcommon.h"

/*
 * Global vars for smbsubscribe
 */
int sock_fd;
Message message = {0};

/* options */
static struct argp_option options[] = {
        {"verbose",  'v', 0,       0, "More Verbose logging"},
        {"port",     'p', "PORT",  0, "Server Port" },
        {"sleep",    's', "num",   0,  "Time to wait till next measurement"},
        { 0 },
};

/* option parsing */
static error_t parse_option(int key, char *arg, struct argp_state *state) {
    CliOptions *options = state->input;

    switch (key) {
        case 'p': options->port = strtol(arg, NULL, 0); break;
        case 'v': options->verbose = true; break;
        case ARGP_KEY_ARG:
            if (state->arg_num == 0) {
                strcpy((char *) server_hostname, arg);
            } else if (state->arg_num == 1) {
                strcpy(message.topic, arg);
            } else {
                argp_usage(state);
            }
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return EXIT_SUCCESS;
}

/* argp parser. */
static struct argp argp = { options, parse_option, args_doc, doc };

int publishMessage(char buffer[MAX_BUFFER_SIZE]) {
    if (socket_serialization(buffer, &message) != 0) {
        return EXIT_FAILURE;
    }
    struct sockaddr_in servaddr = {0};

    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    servaddr.sin_port = htons(PORT);

    sendto(sock_fd, buffer, MAX_BUFFER_SIZE, MSG_CONFIRM,
           (const struct sockaddr *)&servaddr, sizeof(servaddr));

    printf("Published at %s on Topic %s => %s\n",
           timestamp_to_string(message.unix_timestamp),
           message.topic, message.data);
    close(sock_fd);
    return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
    CliOptions cli_options;
    cli_options.port = 8080;
    cli_options.verbose = false;
    cli_options.sleep = 0;
    argp_parse(&argp, argc, argv, 0, 0, &cli_options);

    if (strcmp((const char *) server_hostname, "") == 0) {
        fprintf(stderr, "Provide a server hostname!\n");
        return EXIT_FAILURE;
    }

    if (strcmp(message.topic, "") == 0) {
        fprintf(stderr, "Provide a topic!\n");
        return EXIT_FAILURE;
    }

    if (strstr(message.topic, "#") != NULL) {
        fprintf(stderr, "Can't publish under a Wildcard!\n");
        return EXIT_FAILURE;
    }

    // Example message to publish
    char buffer[MAX_BUFFER_SIZE];
    message.unix_timestamp = time(NULL);
    strcpy(message.data, "Hi, Mom!");
    publishMessage(buffer);
    return EXIT_SUCCESS;
}
