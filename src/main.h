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
int get_ip_from_fqdn(struct Socket *clientSocket, unsigned short port);