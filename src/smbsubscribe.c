#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define MAX_BUFFER_SIZE 1024
#define SERVER_PORT 8080

void subscribeToBroker() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[MAX_BUFFER_SIZE];
    int len = sizeof(servaddr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(0); // Bind to port 0 for dynamic assignment

    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Get the dynamically assigned port
    struct sockaddr_in local_address;
    socklen_t addr_len = sizeof(local_address);
    getsockname(sockfd, (struct sockaddr *)&local_address, &addr_len);
    printf("Subscriber bound to port: %d\n", ntohs(local_address.sin_port));

    while (1) {
        int n = recvfrom(sockfd, (char *)buffer, MAX_BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
        buffer[n] = '\0';
        printf("Message received from broker: %s\n", buffer);
    }

    close(sockfd);
}

int main() {
    subscribeToBroker();

    return 0;
}