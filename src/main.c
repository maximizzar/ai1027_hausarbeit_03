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
void print_connected_clients() {
    printf("Connected clients:\n");
    struct Client* current = client_list;
    while (current != NULL) {
        printf("<$> %s:%d ~ %s\n", current->ip, current->port, role_to_string(current->type));
        current = current->next;
    }
}

/* Socket Serialization and Deserialization */
int socket_serialization(char buffer[BUFFER_SIZE], struct SocketData *data) {
    if (data == NULL) {
        printf("SocketData struct is Null");
        return EXIT_FAILURE;
    }
    if (!data->type) {
        printf("Incorrect type set");
        return EXIT_FAILURE;
    }
    if (strcmp(data->topic, "") == 0) {
        printf("No Topic set");
        return EXIT_FAILURE;
    }

    if (data->data[0] == '\0') {
        sprintf(buffer, "%d \"%s\"", data->type, data->topic);
    }
    sprintf(buffer, "%d \"%s\" \"%s\"", data->type, data->topic, data->data);
    return EXIT_SUCCESS;
}
int socket_deserialization(char buffer[BUFFER_SIZE], struct SocketData *data, bool verbose) {
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

    data->type = strtol(values[0], NULL, 0);
    int return_code = 0;
    if (values[1] != NULL) {
        strcpy(data->topic, values[1]);
    } else {
        if (verbose) {
            printf("Failed Deserialization: No Topic provided");
            return_code = 1;
        }
    }
    if (values[2] != NULL) {
        strcpy(data->data, values[2]);
    } else {
        if (data->type == PUBLISHER) {
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

/* Connect a client via v4 or v6 to the server (broker) */
int connect_to_server(int sock_fd, char *host, unsigned short port,
                      struct addrinfo hints, struct addrinfo *res,
                      struct sockaddr_in6 *sockaddr, struct sockaddr_storage addresses[MAX_ADDRESSES]) {
    int status = -1, address_count = 0;
    struct hostent *server = {0};
    char port_str[6];
    snprintf(port_str, sizeof(port_str), "%hu", port); // Convert unsigned short to string

    if ((status = getaddrinfo(host, port_str, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
        return EXIT_FAILURE;
    }

    struct addrinfo *p;
    for (p = res; p != NULL && address_count < MAX_ADDRESSES; p = p->ai_next) {
        memcpy(&addresses[address_count], p->ai_addr, p->ai_addrlen);
        address_count++;
    }

    /*
     * Loop over addresses provided by getaddrinfo
     * - if an address works the loop breaks with a successful connection
    * - if all addresses don't work, the client closes
    */
    for (int i = 0; i < address_count; i++) {
        char ip_str[INET6_ADDRSTRLEN];
        sockaddr->sin6_port = htons(port);

        /* address in clientSocket.address_list is a v4 address and needs a v4 socket */
        if (addresses[i].ss_family == PF_INET) {
            sockaddr->sin6_family = PF_INET;
            // Create v4 Socket
            if ((sock_fd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
                perror("socket creation failed");
                exit(EXIT_FAILURE);
            }
        }
            /* address in clientSocket.address_list is a v6 address and needs a v6 socket */
        else {
            sockaddr->sin6_family = PF_INET6;
            // Create v6 Socket
            if ((sock_fd = socket(PF_INET6, SOCK_DGRAM, 0)) < 0) {
                perror("socket creation failed");
                exit(EXIT_FAILURE);
            }
        }

        if ((connect(sock_fd, (struct sockaddr *) &addresses[i], sizeof(addresses[i]))) < 0) {
            perror("Connection Failed");
            if (i + 1 == address_count) {
                exit(EXIT_FAILURE);
            }
            continue;
        }
        break;
    }
    return sock_fd;
}

/* Leave loops with SIGINT */
volatile sig_atomic_t interrupted = 0;
void handle_sigint(int sig) {
    interrupted = 1;
}

/* Main Functions */
int broker(struct Arguments arguments) {
    int sock_fd;
    struct sockaddr_in6 server_addr;
    struct sockaddr_in6 client_addr;
    socklen_t addr_len = sizeof(client_addr);

    /* Data */
    char buffer[BUFFER_SIZE];
    struct SocketData data = {0};
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
        printf("Server started. Waiting for clients...\n\n");
    }

    /* Send and recv loop */
    while (!interrupted) {
        /* Receive data from clients */
        recvfrom(sock_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);

        /* Deserialization from socket's buffer into data struct */
        if (socket_deserialization(buffer, &data, arguments.verbose) != 0) {
            if (!arguments.verbose) {
                printf("Deserialization Failed: Ignore packet\n");
            }
            memset(buffer, 0, BUFFER_SIZE);
            memset(&data, 0, sizeof(data));
            continue;
        }

        /* Store client information */
        char client_ip[INET6_ADDRSTRLEN];
        inet_ntop(PF_INET6, &(client_addr.sin6_addr), client_ip, INET6_ADDRSTRLEN);
        int client_port = ntohs(client_addr.sin6_port);

        /* Add new client's to the linked list */
        if (addClient(client_ip, client_port, data.type, data.topic) == 0) {
            if (arguments.verbose) {
                print_connected_clients();
            }
        }

        /* Show received data from the client */
        if (arguments.verbose) {
            if (data.type == SUBSCRIBER) {
                printf("%s %s:%d subscribed topic %s\n",
                       role_to_string(data.type), client_ip, client_port, data.topic);
            } else if (data.type == PUBLISHER) {
                printf("%s %s:%d publishes under topic %s -> %s\n",
                       role_to_string(data.type), client_ip, client_port, data.topic, data.data);
            }
        }

        addToCircularBuffer(measurements, data);
        printCircularBuffer(measurements);

        memset(buffer, 0, BUFFER_SIZE);
        memset(&data, 0, sizeof(data));

        /* BROKER sends data from PUBLISHER to Subscribed clients. */
        struct Client* current = client_list;
        while (current != NULL) {
            if (current->type == SUBSCRIBER) {
                if (strcmp(current->topic, data.topic) == 0) {
                    //printf("%s sends data \"%s\" to %s:%d\n",
                    //       role_to_string(arguments.type),
                    //       data.data, current->ip, current->port);
                }
            }
            current = current->next;
        }
        memset(&data, 0, sizeof data);
    }
    close(sock_fd);
    exit(EXIT_SUCCESS);
}
int publisher(struct Arguments arguments) {
    if (arguments.topic == NULL) {
        printf("A Publisher needs to have a topic\n");
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

    struct SocketData data = {0};

    int i = 0;
    while(!interrupted) {
        data.type = arguments.type;
        strcpy(data.topic, arguments.topic);
        //strcpy(data.data, "Hi Mom!");
        sprintf(data.data, "%d", i);
        i++;


        if (socket_serialization(buffer, &data) == 0) {
            if (arguments.verbose) {
                printf("Send \"%s\" under topic \"%s\" to %s %s:%d\n",
                       data.data, data.topic,
                       role_to_string(BROKER), arguments.host, arguments.port);
            }

            if (sendto(sock_fd, (const char *) buffer, BUFFER_SIZE, 0, (const struct sockaddr *) &address4,
                       sizeof(address4)) < 0) {
                perror("Failed to send msg to the Server\n");
            }
        }
        sleep(1);
        memset(&data, 0, sizeof data);
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
    socklen_t address_length = sizeof(address4);

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

    struct SocketData data = {0};
    data.type = arguments.type;
    strcpy(data.topic, arguments.topic);
    strcpy(data.data, "");

    /* Send desired topic to server */
    if (socket_serialization(buffer, &data) == 0) {
        printf("%s\n", buffer);

        if (sendto(sock_fd, (const char *) buffer, BUFFER_SIZE, 0, (const struct sockaddr *) &address4, address_length) < 0) {
            perror("Failed to send msg to the Server");
        }
    }

    /* recv topic specific data and print it */
    while(!interrupted) {
        recv(sock_fd, buffer, BUFFER_SIZE, 0);
        printf("%s", buffer);
    }
    close(sock_fd);
    exit(EXIT_SUCCESS);
}
int main(int argc, char *argv[]) {
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

    if (arguments.type == BROKER) broker(arguments);
    else if (arguments.type == PUBLISHER) publisher(arguments);
    else if (arguments.type == SUBSCRIBER) subscriber(arguments);
    printf("try -? or --help\n");
    return EXIT_SUCCESS;
}
