//
// Created by maximizzar on 17.07.24.
//

#include "smbsubscribe.h"
#include "smbcommon.h"

Message          message = {0};

/* option parsing */
static error_t parse_option(int key, char *arg, struct argp_state *state) {
    CliOptions *cli_options = state->input;

    switch (key) {
        case 'p': cli_options->port = strtol(arg, NULL, 0); break;
        case 'v': cli_options->verbose = true; break;
        case '4': cli_options->legacy_ip = true; break;
        case ARGP_KEY_ARG:
            if (state->arg_num == 0) {
                strcpy((char *) server_hostname, arg);
            } else if (state->arg_num == 1) {
                strcpy(topic, arg);
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

/*
 * - Initializes a socket for either v6 or v4.
 * - Hostname can be set to name, inet4 and inet6.
 */
int init_socket(const char *hostname, const char *port,
                bool legacy_ip) {
    struct addrinfo  *result, *rp;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
    hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0;          /* Any protocol */

    int s;
    if ((s = getaddrinfo(hostname, port, &hints, &result)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        return EXIT_FAILURE;
    }

    /*
     * getaddrinfo() returns a list of address structures.
     * Try each address until we successfully connect(2).
     * If socket(2) (or connect(2)) fails, we (close the socket
     * and) try the next address.
     */

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        if (legacy_ip && rp->ai_family == PF_INET6)
            continue; //skip v6 if v4 only

        sock_fd = socket(rp->ai_family, rp->ai_socktype,
                         rp->ai_protocol);
        if (sock_fd == -1)
            continue;

        if (connect(sock_fd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;                  /* Success */

        close(sock_fd);
    }

    /* No address succeeded */
    if (rp == NULL) {
        fprintf(stderr, "Could not connect\n");
        return EXIT_FAILURE;
    }

    /* Copy data from struct addrinfo *rp into global hints */
    memcpy(&hints, rp, sizeof (struct addrinfo));
    free(result); /* No longer needed */
    return EXIT_SUCCESS;
}

int subscribe(char buffer[MAX_BUFFER_SIZE], bool verbose) {
    // Get the dynamically assigned port
    if (hints.ai_family == PF_INET6) {
        struct sockaddr_in6 local_address;
        socklen_t addr_len = sizeof(local_address);
        getsockname(sock_fd, (struct sockaddr *)&local_address, &addr_len);
        printf("Subscriber bound to port: %d\n", ntohs(local_address.sin6_port));
    } else if (hints.ai_family == PF_INET) {
        struct sockaddr_in local_address;
        socklen_t addr_len = sizeof(local_address);
        getsockname(sock_fd, (struct sockaddr *)&local_address, &addr_len);
        printf("Subscriber bound to port: %d\n", ntohs(local_address.sin_port));
    } else {
        fprintf(stderr, "No IP Family set!\n");
        return EXIT_FAILURE;
    }

    while (true) {
        memset(buffer, 0, MAX_BUFFER_SIZE);
        if (recvfrom(sock_fd, buffer, MAX_BUFFER_SIZE, MSG_WAITALL,
                     hints.ai_addr, &hints.ai_addrlen) == -1) {
            fprintf(stderr, "Failed to receive Published Message Broker!\n");
            continue;
        }

        if (socket_deserialization(buffer, &message) != 0) {
            continue;
        }

        printf(":: %s \"%s\" = %s\n",
               timestamp_to_string(message.unix_timestamp),
               message.topic, message.data);
    }
}

int main(int argc, char *argv[]) {
    char             buffer[MAX_BUFFER_SIZE] = {0};
    char             port_str[6];
    CliOptions       cli_options;
                     cli_options.port = 8080;
                     cli_options.verbose = false;
                     cli_options.legacy_ip = false;
    argp_parse(&argp, argc, argv, 0, 0, &cli_options);
    sprintf(port_str, "%hu", cli_options.port);

    if (strcmp((const char *) server_hostname, "") == 0) {
        fprintf(stderr, "Provide a server hostname!\n");
        return EXIT_FAILURE;
    }

    if (strcmp(topic, "") == 0) {
        fprintf(stderr, "Provide a topic!\n");
        return EXIT_FAILURE;
    }

    /*
     * inits a socket with v6 or v4.
     * returns 0 if successful else 1
     */
    if (init_socket((const char *) server_hostname, port_str,
                    cli_options.legacy_ip) != 0) {
        return EXIT_FAILURE;
    }

    message.unix_timestamp = time(NULL);
    strcpy(message.topic, topic);

    if (socket_serialization(buffer, &message)) {
        return EXIT_FAILURE;
    }

    if (sendto(sock_fd, buffer, MAX_BUFFER_SIZE, MSG_CONFIRM,
               hints.ai_addr, hints.ai_addrlen) < 0) {
        perror("Subscribing to topic on broker failed!");
        return EXIT_FAILURE;
    }
    subscribe(buffer, cli_options.verbose);
    return EXIT_SUCCESS;
}
