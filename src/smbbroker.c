//
// Created by maximizzar on 17.07.24.
//

#include "smbbroker.h"
#include "smbcommon.h"

#include <argp.h>

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
        default:
            return ARGP_ERR_UNKNOWN;
    }
    return EXIT_SUCCESS;
}

/* argp parser. */
static struct argp argp = { options, parse_option, args_doc, doc };

int main(int argc, char *argv[]) {
    CliOptions cli_options;
    cli_options.port = 8080;
    cli_options.verbose = false;
    argp_parse(&argp, argc, argv, 0, 0, &cli_options);

    int sockfd, opt = 1;
    struct sockaddr_in servaddr = {0}, cliaddr = {0};
    char buffer[MAX_BUFFER_SIZE] = {0};
    Subscriber subscribers[SUBSCRIBERS_MAX];
    CircularBuffer circularBuffer = *create_circular_buffer(CIRCULARBUFFERSIZE);
    int subscribers_current = 0;

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    //set sockfd to allow multiple connections,
    //this is just a good habit; it will work without this
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,
                   sizeof(opt)) < 0 ) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind socket to port
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    ssize_t n;
    socklen_t len;
    len = sizeof(cliaddr);

    while(true) {
        Message message;
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
            add_to_circular_buffer(&circularBuffer, &message);
        } else {
            // Check if subscriber is new
            int new_subscriber = 1;
            for (int i = 0; i < subscribers_current; i++) {
                if (strcmp(inet_ntoa(cliaddr.sin_addr), subscribers[i].address) == 0 && ntohs(cliaddr.sin_port) == subscribers[i].port) {
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
                    strcpy(subscribers[subscribers_current].address, inet_ntoa(cliaddr.sin_addr));
                    subscribers[subscribers_current].port = ntohs(cliaddr.sin_port);
                    strcpy(subscribers[subscribers_current].topic, message.topic);
                    subscribers_current++;
                    printf(":: New subscriber added: %s:%d => %s\n",
                           subscribers[subscribers_current - 1].address,
                           subscribers[subscribers_current - 1].port,
                           subscribers[subscribers_current - 1].topic);

                    /*
                     * Loop over all entries in the circular buffer
                     * and send the subscriber all missed data for its topic (WIP)
                     */
                    continue;
                    for (int j= circularBuffer.index; j < circularBuffer.size + circularBuffer.index; j++) {
                        int index = j % circularBuffer.size;
                        memset(buffer, 0, MAX_BUFFER_SIZE);
                        socket_serialization(buffer, &circularBuffer.message[index]);
                        sendto(sockfd, (char *)buffer, MAX_BUFFER_SIZE, MSG_WAITALL,
                               (struct sockaddr *)&cliaddr, len);
                    }
                } else {
                    if (cli_options.verbose) {
                        fprintf(stderr, ":: Max subscribers reached. Cannot add new subscriber!\n");
                    }
                }
                continue;
            }
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
            char log[200];
            memset(buffer, 0, MAX_BUFFER_SIZE);
            socket_serialization(buffer, &message);

            if (cli_options.verbose) {
                printf(":: :: Subscriber \"%s:%d\" requested Topic %s:\n",
                       subscribers[i].address, subscribers[i].port, subscribers[i].topic);
            }
            if (match_topic(subscribers[i].topic, message.topic)) {
                struct sockaddr_in sub;
                // Convert IP address from string to s_addr
                if (inet_aton(subscribers[i].address, &sub.sin_addr) == 0) {
                    perror("Invalid IP address");
                    exit(EXIT_FAILURE);
                }
                sub.sin_port = htons(subscribers[i].port);

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
            printf(":: SUCCESS!\n\n");
        }
    }
    return EXIT_SUCCESS;
}
