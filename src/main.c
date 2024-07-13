//
// Created by maximizzar on 02.07.24.
//

#include "main.h"
#include "server.h"
#include "client.h"

const char *argp_program_version = "SMB 1.0.0";
const char *argp_program_bug_address = "<mail@maximizzar.de>";

/* Program documentation. */
static char doc[] = "SMB: Simple Message Broker";

/* options */
static struct argp_option options[] = {
        {"role",    'r', "ROLE",  0, "BROKER, PUB, SUB" },
        {"topic",   't', "TOPIC", 0, "Topic to write to"},
        {"port",    'p', "PORT",  0, "Server Port" },
        {"host",    'h', "HOST",  0, "Host"},
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

// Function to add a new client to the linked list
void addClient(char* ip, int port) {
    struct Client* current = client_list;

    // Check if the client already exists in the list
    while (current != NULL) {
        if (strcmp(current->ip, ip) == 0 && current->port == port) {
            return; /* Information already exists */
        }
        current = current->next;
    }

    struct Client* newClient = (struct Client*)malloc(sizeof(struct Client));
    strcpy(newClient->ip, ip);
    newClient->port = port;
    newClient->next = client_list;
    client_list = newClient;
}

int broker(struct Arguments arguments) {
    int sock_fd;
    struct sockaddr_in6 server_addr;
    struct sockaddr_in6 client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE];

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

    // Bind the socket to a specific IP address and port
    server_addr.sin6_family = PF_INET6;
    server_addr.sin6_addr = in6addr_any;
    server_addr.sin6_port = htons(12345);
    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Failed to bind");
        close(sock_fd);
        exit(EXIT_FAILURE);
    }

    if (arguments.verbose) {
        printf("Server started. Waiting for clients...\n");
    }

    while (true) {
        // Receive data from clients
        recvfrom(sock_fd, buffer, 1024, 0, (struct sockaddr *)&client_addr, &addr_len);

        // Determine client type

        // Store client information
        char client_ip[INET6_ADDRSTRLEN];
        inet_ntop(PF_INET6, &(client_addr.sin6_addr), client_ip, INET6_ADDRSTRLEN);
        int client_port = ntohs(client_addr.sin6_port);

        // Add client to the linked list
        addClient(client_ip, client_port);

        // Print out the list of connected clients
        if (arguments.verbose) {
            printf("Connected clients:\n");
        }

        struct Client* current = client_list;
        while (current != NULL) {
            if (arguments.verbose) {
                printf("Information %s:%d - Type %d\n", current->ip, current->port, current->type);
            }
            current = current->next;
        }

        // Process the received data from the client
        printf("Received data from client %s:%d: %s\n", client_ip, client_port, buffer);
    }
    return EXIT_SUCCESS;
}

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

int publisher(struct Arguments arguments) {
    if (arguments.topic == NULL) {
        printf("A Publisher needs to have a topic\n");
        exit(1);
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

    //if ((sock_fd = connect_to_server(sock_fd, arguments.host, arguments.port, hints, res, &address, addresses)) < 0) {
    //    perror("Failed to Connect to the Server");
    //    exit(EXIT_FAILURE);
    //}

    //temp v4 only testing
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Failed to create Socket");
        exit(EXIT_FAILURE);
    }

    address4.sin_family = PF_INET;
    address4.sin_port = htons(arguments.port);
    address4.sin_addr.s_addr = INADDR_ANY;

    char *hello = "Hi Mom!";
    while(true) {
        if (sendto(sock_fd, (const char *)"hello", strlen(hello), 0, (const struct sockaddr *)&address, sizeof(address)) < 0) {
            perror("Failed to send msg to the Server");
        }
        sleep(1);
    }
    return EXIT_SUCCESS;
}
int subscriber(struct Arguments arguments) {
    return EXIT_SUCCESS;
}

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
    return EXIT_FAILURE;
}
