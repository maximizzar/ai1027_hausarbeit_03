//
// Created by maximizzar on 17.07.24.
//

#include "smbsubscribe.h"
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
        { 0 },
};

/* option parsing */
static error_t parse_option(int key, char *arg, struct argp_state *state) {
    CliOptions *options = state->input;

    switch (key) {
        case 'p': options->port = strtol(arg, NULL, 0); break;
        case 'v': options->verbose = true; break;
        case ARGP_KEY_ARG:
            if (state->arg_num > 1) {
                printf("Too many arguments.\n");
                return EXIT_FAILURE;
            }
            if (state->arg_num == 0) {
                strcpy((char *) server_hostname, arg);
            } else {
                strcpy(message.topic, arg);
            }
            return EXIT_SUCCESS;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return EXIT_SUCCESS;
}

/* argp parser. */
static struct argp argp = { options, parse_option, args_doc, doc };

void subscribeToBroker() {
    struct sockaddr_in servaddr = {0}, cliaddr= {0};
    char buffer[MAX_BUFFER_SIZE];
    int len = sizeof(servaddr);

    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    servaddr.sin_port = htons(PORT);

    //const char *message = "Hello, Server! Give me data.";

    message.unix_timestamp = time(NULL);
    strcpy(message.topic, "Hello World");
    socket_serialization(buffer, &message);
    if (sendto(sock_fd, buffer, MAX_BUFFER_SIZE, MSG_CONFIRM,
               (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Subscribing to topic on broker failed!");
    }

    // Get the dynamically assigned port
    struct sockaddr_in local_address;
    socklen_t addr_len = sizeof(local_address);
    getsockname(sock_fd, (struct sockaddr *)&local_address, &addr_len);
    printf("Subscriber bound to port: %d\n", ntohs(local_address.sin_port));

    while (1) {
        int n = recvfrom(sock_fd, (char *)buffer, MAX_BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
        buffer[n] = '\0';
        printf("Message received from broker: %s\n", buffer);
    }
    close(sock_fd);
}

int main(int argc, char *argv[]) {
    CliOptions cli_options;
    cli_options.port = 8080;
    cli_options.verbose = false;
    argp_parse(&argp, argc, argv, 0, 0, &cli_options);

    subscribeToBroker();

    return EXIT_SUCCESS;
}
