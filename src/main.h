//
// Created by maximizzar on 02.07.24.
//

#include <netinet/in.h>
#include <netdb.h>

#define SOCKET_TYPE SOCK_DGRAM
#define BUFFER_SIZE 8192
#define MAX_ADDRESSES 8

/* Socket struct */
struct Socket {
    int broker_fd, client_fd, opt, status, address_count;
    struct sockaddr_in6 broker_address, client_address;
    struct sockaddr_storage address_list[MAX_ADDRESSES];
    struct addrinfo hints, *res;
    socklen_t broker_address_length, client_address_length;

    char buffer[BUFFER_SIZE];
};

/* cli arguments */
struct Arguments {
    char *role, *topic, *host;
    unsigned short port;
};

/* Get IP from FQDN */
int get_ip_from_fqdn(struct Socket clientSocket, unsigned short port) {
    /*
     * Loop over addresses in clientSocket.address_list provided by getaddrinfo
     * - if an address works the loop breaks with a successful connection
     * - if all addresses don't work, the client closes
     */
    for (int i = 0; i < clientSocket.address_count; i++) {
        char ip_str[INET6_ADDRSTRLEN];
        memset(&clientSocket.client_address, 0, sizeof clientSocket.client_address);
        clientSocket.client_address.sin6_port = htons(port);

        /* address in clientSocket.address_list is a v4 address and needs a v4 socket */
        if (clientSocket.address_list[i].ss_family == PF_INET) {
            clientSocket.client_address.sin6_family = PF_INET;
            // Create v4 Socket
            if ((clientSocket.client_fd = socket(PF_INET, SOCKET_TYPE, 0)) < 0) {
                perror("socket creation failed");
                exit(EXIT_FAILURE);
            }
        }
            /* address in clientSocket.address_list is a v6 address and needs a v6 socket */
        else {
            clientSocket.client_address.sin6_family = PF_INET6;
            // Create v6 Socket
            if ((clientSocket.client_fd = socket(PF_INET6, SOCKET_TYPE, 0)) < 0) {
                perror("socket creation failed");
                exit(EXIT_FAILURE);
            }
        }

        if ((clientSocket.status = connect(clientSocket.client_fd, (struct sockaddr *) &clientSocket.address_list[i], sizeof(clientSocket.address_list[i]))) < 0) {
            perror("Connection Failed");
            if (i + 1 == clientSocket.address_count) {
                exit(EXIT_FAILURE);
            }
            continue;
        }
        break;
    }
    return EXIT_SUCCESS;
}