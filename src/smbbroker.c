//
// Created by maximizzar on 17.07.24.
//

#include "smbbroker.h"
#include "smbcommon.h"

/* option parsing */
static error_t parse_option(int key, char *arg, struct argp_state *state) {
    CliOptions *cli_options = state->input;

    switch (key) {
        case 'p': cli_options->port = strtol(arg, NULL, 0); break;
        case 'v': cli_options->verbose = true; break;
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return EXIT_SUCCESS;
}

/* argp parser. */
static struct argp argp = { options, parse_option, args_doc, doc };
/*
 * - Initializes a socket for v6 only
 * - Due to
 */

int main(int argc, char *argv[]) {
    int              reuse_address = 1, v6_only = 0;
    char             buffer[MAX_BUFFER_SIZE] = {0};
    char             port_str[6];
    CliOptions       cli_options;
    cli_options.port = 8080;
    cli_options.verbose = false;
    argp_parse(&argp, argc, argv, 0, 0, &cli_options);

    struct sockaddr_in6 servaddr = {0}, cliaddr = {0};
    Subscriber subscribers[SUBSCRIBERS_MAX];
    int subscribers_current = 0;

    // Create UDP socket
    if ((sockfd = socket(PF_INET6, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    /* allows to Reuse the Address */
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse_address,
                   sizeof(reuse_address)) < 0 ) {
        perror("Failed to Set socket option");
        exit(EXIT_FAILURE);
    }

    /* Allows for v6 mapped v4 in a v6 socket */
    if (setsockopt(sockfd, IPPROTO_IPV6, IPV6_V6ONLY, &v6_only, sizeof(v6_only)) == -1) {
        perror("Failed to Set socket option");
        exit(EXIT_FAILURE);
    }

    servaddr.sin6_family = PF_INET6;
    servaddr.sin6_addr = in6addr_any;
    servaddr.sin6_port = htons(cli_options.port);

    // Bind socket to port
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    ssize_t n;
    socklen_t len;
    len = sizeof(cliaddr);

    while(true) {
        Message message = {0};
        memset(buffer, 0, MAX_BUFFER_SIZE);
        n = recvfrom(sockfd, (char *)buffer, MAX_BUFFER_SIZE, MSG_WAITALL,
                     (struct sockaddr *)&cliaddr, &len);
        buffer[n] = '\0';

        if (socket_deserialization(buffer, &message) != 0) {
            continue;
        }

        /* if data is present, a publisher is assumed */
        if (strcmp(message.data, "") != 0) {
            if (strstr(message.topic, "#") != NULL) {
                continue;
            }
            printf("Publication at %s on Topic %s => %s\n",
                   timestamp_to_string(message.unix_timestamp),
                   message.topic, message.data);
        } else {
            // Check if subscriber is new
            int new_subscriber = 1;
            char client_ip[INET6_ADDRSTRLEN];

            for (int i = -1; i < subscribers_current; i++) {
                inet_ntop(PF_INET6, &cliaddr.sin6_addr, client_ip, sizeof(client_ip));
                if (strcmp(client_ip, subscribers[i].address) == 0 &&
                    ntohs(cliaddr.sin6_port) == subscribers[i].port) {
                    new_subscriber = 0;
                    break;
                }
            }

            /*
             * Add Subscribers if they don't already exist.
             * To prevent resending the last element,
             * continue after adding Subscriber to the Array.
             */
            if (new_subscriber) {
                if (subscribers_current < SUBSCRIBERS_MAX) {
                    memcpy(&subscribers[subscribers_current].addr, &cliaddr, sizeof (struct sockaddr));
                    strcpy(subscribers[subscribers_current].address, client_ip);
                    subscribers[subscribers_current].port = ntohs(cliaddr.sin6_port);
                    strcpy(subscribers[subscribers_current].topic, message.topic);
                    subscribers_current++;
                    printf(":: New subscriber added: [%s]:%d => %s\n",
                           subscribers[subscribers_current - 1].address,
                           subscribers[subscribers_current - 1].port,
                           subscribers[subscribers_current - 1].topic);
                    }
                } else {
                    if (cli_options.verbose) {
                        fprintf(stderr, ":: Max subscribers reached. Cannot add new subscriber!\n");
                    }
                }
            continue;
        }
        if (subscribers_current == 0) {
            if (cli_options.verbose) {
                fprintf(stderr, ":: No Subscribers connected. Skip Sending Messages!\n");
            }
            continue;
        }

        /* loop over all know subscribers */
        printf(":: Start to send data to interested Subscribers!\n");
        for (int i = 0; i < subscribers_current; ++i) {
            memset(buffer, 0, MAX_BUFFER_SIZE);
            socket_serialization(buffer, &message);

            if (cli_options.verbose) {
                printf(":: :: Subscriber \"[%s]:%d\" requested Topic %s:\n",
                       subscribers[i].address, subscribers[i].port, subscribers[i].topic);
            }
            if (match_topic(subscribers[i].topic, message.topic)) {
                struct sockaddr_in6 sub;
                memcpy(&sub, &subscribers[i], sizeof(sub));
                if (sendto(sockfd, buffer, MAX_BUFFER_SIZE, 0, (const struct sockaddr *)&sub, sizeof(sub)) < 0) {
                    perror("Failed to send message!");
                } else {
                    if (cli_options.verbose) {
                        printf(":: :: :: Send %s under topic %s\n", message.data, message.topic);
                    }
                }
            } else {
                if (cli_options.verbose) {
                    printf(":: ");
                }
                continue;
            }
        }
        printf(":: SUCCESS!\n\n");
    }
    return EXIT_SUCCESS;
}
