//
// Created by maximizzar on 17.07.24.
//

#include "smbsubscribe.h"
#include "smbcommon.h"


#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

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
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    servaddr.sin_port = htons(PORT);


    const char *message = "Hello, Server! Give me data.";
    sendto(sockfd, message, strlen(message), MSG_CONFIRM,
           (const struct sockaddr *)&servaddr, sizeof(servaddr));

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

    return EXIT_SUCCESS;
}
