//
// Created by maximizzar on 02.07.24.
//

#include "main.h"

/* options */
static struct argp_option options[] = {
        {0,          0,   0,       0, "App Settings:"},
        {"app-type", 'a', "TYPE",  0, "BROKER, PUB, SUB" },
        {"topic",    't', "TOPIC", 0, "Topic to Read / Write from / to"},
        {"verbose",  'v', 0,       0, "More Verbose logging"},

        {0,          0,   0,       0,"Connection Settings:"},
        {"host",     'h', "HOST",  0, "Host"},
        {"port",     'p', "PORT",  0, "Server Port" },

        { 0 },
};

/* option parsing */
static error_t parse_option(int key, char *arg, struct argp_state *state) {
    struct Arguments *arguments = state->input;

    switch (key) {
        case 'a':
            if (strcmp(arg, "BROKER") == 0) {
                arguments->type = BROKER;
            } else if (strcmp(arg, "PUB") == 0) {
                arguments->type = PUBLISHER;
            } else if(strcmp(arg, "SUB") == 0) {
                arguments->type = SUBSCRIBER;
            } else {
                printf("ERROR: Unknown type %s", arg);
                return EXIT_FAILURE;
            }
            break;
        case 't': arguments->topic = arg; break;
        case 'p': arguments->port = strtol(arg, NULL, 0); break;
        case 'h': arguments->host = arg; break;
        case 'v': arguments->verbose = true; break;
        default: return ARGP_ERR_UNKNOWN;
    }
    return EXIT_SUCCESS;
}

/* argp parser. */
static struct argp argp = { options, parse_option, NULL, doc };

/* Helper to easily print stuff */
char* role_to_string(enum Type role) {
    switch(role) {
        case BROKER: return "BROKER";
        case PUBLISHER: return "PUBLISHER";
        case SUBSCRIBER: return "SUBSCRIBER";
        default:
            return "";
    }
}
char* timestamp_to_string(time_t timestamp) {
    struct tm *local_time = localtime(&timestamp);
    return asctime(local_time);
}

void print_connected_clients() {
    printf("Connected clients:\n");
    struct Client* current = client_list;
    while (current != NULL) {
        printf("<$> %s:%d ~ %s\n", current->ip, current->port, role_to_string(current->type));
        current = current->next;
    }
}

/* Socket Serialization and Deserialization */
int socket_serialization(char buffer[BUFFER_SIZE], struct SocketData *socket_data) {
    if (socket_data == NULL) {
        printf("SocketData struct is Null");
        return EXIT_FAILURE;
    }
    if (!socket_data->type) {
        printf("Incorrect type set");
        return EXIT_FAILURE;
    }
    if (strcmp(socket_data->topic, "") == 0) {
        printf("No Topic set");
        return EXIT_FAILURE;
    }

    if (socket_data->measurement.data[0] == '\0') {
        sprintf(buffer, "\"%d\" \"%s\"", socket_data->type, socket_data->topic);
    }
    sprintf(buffer, "\"%d\" \"%s\" \"%ld\" \"%s\"", socket_data->type, socket_data->topic, socket_data->measurement.timestamp, socket_data->measurement.data);
    return EXIT_SUCCESS;
}
int socket_deserialization(char buffer[BUFFER_SIZE], struct SocketData *socket_data, bool verbose) {
    int count = 0;

    // Count the number of values in the buffer
    for (int i = 0; buffer[i] != '\0'; i++) {
        if (buffer[i] == '"') {
            count++;
        }
    }
    char** values = (char**)malloc(count * sizeof(char*));

    // Parse the values based on the count
    char* token = strtok(buffer, "\"");
    int index = 0;
    while (token != NULL) {
        if (strcmp(token, " ") != 0 && strcmp(token, "") != 0) {
            values[index] = strdup(token);
            index++;
        }
        token = strtok(NULL, "\"");
    }

    if (strcmp(values[0], "0") != 0) {
        socket_data->type = strtol(values[0], NULL, 0);
    }
    int return_code = 0;
    if (strcmp(values[1], "0") != 0) {
        strcpy(socket_data->topic, values[1]);
    } else {
        if (verbose) {
            printf("Failed Deserialization: No Topic provided");
            return_code = 1;
        }
    }
    if (strcmp(values[2], "0") != 0) {
        socket_data->measurement.timestamp = (time_t) strtol(values[2], NULL, 0);
        strcpy(socket_data->measurement.data, values[3]);
    } else {
        if (socket_data->type == PUBLISHER) {
            if (verbose) {
                printf("Failed Deserialization: A Publisher must send data");
                return_code = 1;
            } else  {
            }
        }
    }
    // Free allocated memory
    for (int i = 0; i < count; i++) {
        free(values[i]);
        values[i] = NULL;
    }
    free(values);
    return return_code;
}

/* Function to add a new client to the linked list */
int addClient(char* ip, int port, enum Type type, char* topic) {
    struct Client* current = client_list;

    // Check if the client already exists in the list
    while (current != NULL) {
        if (strcmp(current->ip, ip) == 0 && current->port == port) {
            return EXIT_FAILURE; /* Information already exists */
        }
        current = current->next;
    }

    struct Client* newClient = (struct Client*)malloc(sizeof(struct Client));
    if ((newClient->sockfd = socket(PF_INET6, SOCK_DGRAM, 0)) < 0) {
        perror("Failed to Create Socket");
        return EXIT_FAILURE;
    }
    strcpy(newClient->ip, ip);
    newClient->port = port;
    newClient->type = type;

    newClient->topic = (char*)malloc(strlen(topic) + 1);
    if (newClient->topic != NULL) {
        strcpy(newClient->topic, topic);
    }
    newClient->next = client_list;
    client_list = newClient;
    return EXIT_SUCCESS;
}

/* Leave loops with SIGINT */
volatile sig_atomic_t interrupted = 0;
void handle_sigint(int sig) {
    interrupted = 1;
}

/* WIP (will probably never work :/) */
int create_client_socket(const char *host, unsigned short port, struct sockaddr_in6 *server_addr) {
    char port_str[6];
    sprintf(port_str, "%u", port);
    struct addrinfo hints, *res, *p;
    int sock_fd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;  // Use AF_INET6 to force IPv6
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_NUMERICSERV;

    if (getaddrinfo(host, port_str, &hints, &res) != 0) {
        fprintf(stderr, "Error getting address info\n");
        return -1;
    }

    // Attempt to create socket and connect
    for (p = res; p != NULL; p = p->ai_next) {
        sock_fd = socket(p->ai_family, SOCK_DGRAM, 0);
        if (sock_fd == -1) {
            continue;
        }

        if (connect(sock_fd, p->ai_addr, p->ai_addrlen) != -1) {
            char ip[INET6_ADDRSTRLEN];

            inet_ntop(p->ai_family, &(((struct sockaddr_in*)(p->ai_addr))->sin_addr), ip, sizeof(ip));
            printf("Connected to %s via %s\n", host, ip);

            // Copy server address to the provided struct
            memcpy(server_addr, p->ai_addr, p->ai_addrlen);

            freeaddrinfo(res);
            return sock_fd;
        }
        close(sock_fd);
    }
    freeaddrinfo(res);
    return -1;
}

bool match_topic(char *sub, char *data) {
    /*
     * - sub is a topic from a sub in the linked list
     * - data is a topic from a data point in the Circular Buffer
     */

    /* check if sub has a wildcard */
    for (int i = 0; sub[i] != '\0'; i++) {
        if (sub[i] == '#') {
            if (sub[i + 1] != '\0') {
                return false; // ignore incorrectly formated topics
            }
            if (strncmp(sub, data, i - 1) != 0) {
                return false; // if sub does not data's sub-string
            }
            return true;
        }
    }
    // if no wildcard found, the whole topics get matched
    if (strcmp(sub, data) != 0) {
        return false;
    }
    return true;
}

/* Main Functions */
int broker(struct Arguments arguments) {
    /* Check for unnecessary options */
    if (arguments.topic) {
        printf("The %s does not need an Topic. Consider removing the topic option.\n",
               role_to_string(arguments.type));
    }

    if (arguments.verbose) {
        printf("%s is in verbose mode!\n", role_to_string(BROKER));
    } else {
        printf("%s is not in verbose mode!\n", role_to_string(BROKER));
    }

    /* Data to create the socket */
    int sock_fd;
    fd_set readfds;
    int max_sd, activity, sd;

    struct sockaddr_in6 server_addr = {0};
    struct sockaddr_in6 client_addr = {0};
    socklen_t addr_len = sizeof(client_addr);

    /* Data buffer for socket communication */
    char socket_buffer[BUFFER_SIZE];
    struct SocketData socket_data = {0};
    CircularBuffer* measurements = createCircularBuffer(10);

    /* Create Socket */
    if ((sock_fd = socket(PF_INET6, SOCK_DGRAM, 0)) < 0) {
        perror("Failed to create Socket");
        exit(EXIT_FAILURE);
    }

    /* use setsockopt to get ipv4 working over ipv6 socket */
    if (setsockopt(sock_fd, IPPROTO_IPV6, IPV6_V6ONLY, 0, sizeof(0)) < 0) {
        perror("Error setting socket option");
        exit(EXIT_FAILURE);
    }

    /* Bind the socket to a specific IP address and port */
    server_addr.sin6_family = PF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(arguments.port);
    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    if (arguments.verbose) {
        printf("Server started. Waiting for clients...\n");
    }

    /* Send and Recv loop */
    while (!interrupted) {
        FD_ZERO(&readfds);
        FD_SET(sock_fd, &readfds);
        max_sd = sock_fd;

        // Add client sockets to the set
        for (struct Client* current = client_list; current != NULL; current = current->next) {
            sd = current->sockfd;
            FD_SET(sd, &readfds);
            if (sd > max_sd) {
                max_sd = sd;
            }
        }

        // Use select to monitor socket activity
        //if (activity = select(max_sd + 1, &readfds, NULL, NULL, NULL) < 0) {
        //    perror("Select error");
        //    exit(EXIT_FAILURE);
        //}

        /* Handle incoming data from clients */
        if (FD_ISSET(sock_fd, &readfds)) {
            // Accept new client connections and add them to the linked list

            /* Receive socket_data from clients */
            recvfrom(sock_fd, socket_buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);

            /* Deserialization from socket's socket_buffer into socket_data struct */
            if (socket_deserialization(socket_buffer, &socket_data, arguments.verbose) != 0) {
                if (!arguments.verbose) {
                    printf("Deserialization Failed: Ignore packet\n");
                }
                memset(socket_buffer, 0, BUFFER_SIZE);
                memset(&socket_data, 0, sizeof(socket_data));
                continue;
            }

            /* Store client information */
            char client_ip[INET6_ADDRSTRLEN];
            inet_ntop(PF_INET6, &(client_addr.sin6_addr), client_ip, INET6_ADDRSTRLEN);
            int client_port = ntohs(client_addr.sin6_port);

            /* Add new client's to the linked list */
            if (addClient(client_ip, client_port, socket_data.type, socket_data.topic) == 0) {
                if (arguments.verbose) {
                    print_connected_clients();
                }
            }

            /* Show received socket_data */
            if (arguments.verbose) {
                if (socket_data.type == SUBSCRIBER) {
                    printf("%s: %s:%d subscribed topic %s\n",
                           role_to_string(socket_data.type), client_ip, client_port, socket_data.topic);
                } else if (socket_data.type == PUBLISHER) {
                    printf("%s: %s:%d publishes under topic %s at %s",
                           role_to_string(socket_data.type), client_ip, client_port, socket_data.topic,
                           timestamp_to_string(socket_data.measurement.timestamp));
                }
            }

            /*
             * If socket_data came from a Publisher, save it in the CircularBuffer measurements
             * Afterward clear socket buffer and socket data struct
             */
            add_measurement(measurements, socket_data);
            memset(socket_buffer, 0, BUFFER_SIZE);
            memset(&socket_data, 0, sizeof(socket_data));
        }

        /* Handle data from existing clients */
        for (struct Client* current = client_list; current != NULL; current = current->next) {
            sd = current->sockfd;
            if (FD_ISSET(sd, &readfds) && current->type == SUBSCRIBER) {
                // Read data from client socket
                // Process the data and send responses back to subscribers

                //for (int i = 0; measurements->array->topic[i]; i++) {
                //    if (match_topic(current->topic, &measurements->array->topic[i])) {
                        // if topic matches send data to subs.
                        // due to that not working, it's in testing below!
                //    }
                //}

                char response[] = "Message received!";
                sendto(sd, response, BUFFER_SIZE, 0, (const struct sockaddr *) &current->address,
                        sizeof(current->address));
                printf("Message send\n");
            } else {
                if (arguments.verbose) {
                    printf("%s: No %s, skip sending data\n",
                           role_to_string(BROKER), role_to_string(SUBSCRIBER));
                }
            }
        }
    }
    close(sock_fd);
    exit(EXIT_SUCCESS);
}
int publisher(struct Arguments arguments) {
    if (arguments.topic == NULL) {
        fprintf(stderr, "A %s needs to have a topic\n", role_to_string(arguments.type));
        exit(EXIT_FAILURE);
    }

    if (strcmp(arguments.topic, "#") == 0) {
        fprintf(stderr, "A %s can't have an wildcard on it's root", role_to_string(arguments.type));
        exit(EXIT_FAILURE);
    }

    int sock_fd = 0;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in address4 = {0};
    struct sockaddr_in6 address = {0};
    struct sockaddr_storage addresses[MAX_ADDRESSES] = {0};
    struct addrinfo hints = {0}, *res = 0;
    socklen_t address_length = sizeof(address);

    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_NUMERICSERV;

    //temp v4 only testing
    if ((sock_fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Failed to create Socket");
        exit(EXIT_FAILURE);
    }

    address4.sin_family = PF_INET;
    address4.sin_port = htons(arguments.port);
    address4.sin_addr.s_addr = INADDR_ANY;

    struct SocketData socket_data = {0};

    int i = 0;
    while(!interrupted) {
        socket_data.type = arguments.type;
        strcpy(socket_data.topic, arguments.topic);
        time_t now;
        now = time(NULL);
        printf("%s", timestamp_to_string(now));
        socket_data.measurement.timestamp = now;
        sprintf(socket_data.measurement.data, "%d", i);
        i++;

        if (socket_serialization(buffer, &socket_data) == 0) {
            if (arguments.verbose) {
                printf("Send \"%s\" under topic \"%s\" to %s %s:%d\n",
                       socket_data.measurement.data, socket_data.topic,
                       role_to_string(BROKER), arguments.host, arguments.port);
            }

            if (sendto(sock_fd, (const char *) buffer, BUFFER_SIZE, 0, (const struct sockaddr *) &address4,
                       sizeof(address4)) < 0) {
                perror("Failed to send msg to the Server\n");
            }
        }
        sleep(3);
        memset(&socket_data, 0, sizeof socket_data);
    }
    exit(EXIT_SUCCESS);
}
int subscriber(struct Arguments arguments) {
    if (arguments.topic == NULL) {
        printf("A Subscriber needs to have a topic\n");
        exit(EXIT_FAILURE);
    }

    int sock_fd = 0;
    char buffer[BUFFER_SIZE];
    struct sockaddr_in address4 = {0};
    struct sockaddr_in6 address = {0};
    struct sockaddr_storage addresses[MAX_ADDRESSES] = {0};
    struct addrinfo hints = {0}, *res = 0;
    socklen_t address_length = sizeof(address);

    hints.ai_family = PF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_NUMERICSERV;

    //temp v4 only testing
    if ((sock_fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Failed to create Socket");
        exit(EXIT_FAILURE);
    }

    address4.sin_family = PF_INET;
    address4.sin_port = htons(arguments.port);
    address4.sin_addr.s_addr = INADDR_ANY;

    struct SocketData socket_data = {0};
    socket_data.type = arguments.type;
    strcpy(socket_data.topic, arguments.topic);

    /* Send desired topic to server */
    if (socket_serialization(buffer, &socket_data) == 0) {
        if (sendto(sock_fd, buffer, BUFFER_SIZE, 0, (const struct sockaddr *) &address4, sizeof(address4)) < 0) {
            perror("Failed to send msg to the Server");
            exit(EXIT_FAILURE);
        }
    }

    /* recv topic specific socket_data and print it */
    while(!interrupted) {
        memset(buffer, 0, BUFFER_SIZE);
        recv(sock_fd, buffer, BUFFER_SIZE, MSG_WAITALL);
        printf("MSG: %s\n", buffer);

        socket_deserialization(buffer, &socket_data, arguments.verbose);
        printf("%s[%s] %s", timestamp_to_string(socket_data.measurement.timestamp),
               socket_data.topic, socket_data.measurement.data);
    }
    close(sock_fd);
    exit(EXIT_SUCCESS);
}
int main(int argc, char *argv[]) {
    /* SIGINT Handling for infinite-loops */
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    /* Set arg defaults and then parse cli-arg into struct */
    struct Arguments arguments;
    memset(&arguments, 0, sizeof arguments);
    arguments.type = -1;
    arguments.host = "::1";
    arguments.port = 8080;
    arguments.verbose = false;
    argp_parse (&argp, argc, argv, 0, 0, &arguments);

    /*
     * Call correct function based of the app-type
     * Also provide users options via the arguments struct
     * If an app-type requires options,
     * it will early return and tell the user.
     * */
    if (arguments.type == BROKER) broker(arguments);
    else if (arguments.type == PUBLISHER) publisher(arguments);
    else if (arguments.type == SUBSCRIBER) subscriber(arguments);

    /* Or tell the user to consider reading the help page */
    printf("try -? or --help\n");
    return EXIT_SUCCESS;
}
