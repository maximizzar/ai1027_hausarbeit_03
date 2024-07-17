//
// Created by maximizzar on 17.07.24.
//

#include "smbsubscribe.h"
#include "smbcommon.h"

void subscribeToBroker() {
    int sockfd;
    struct sockaddr_in servaddr = {0}, cliaddr= {0};
    char buffer[MAX_BUFFER_SIZE];
    int len = sizeof(servaddr);

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    servaddr.sin_port = htons(PORT);

    //const char *message = "Hello, Server! Give me data.";
    Message message = {0};
    message.unix_timestamp = time(NULL);
    strcpy(message.topic, "Hello World");
    socket_serialization(buffer, &message);
    if (sendto(sockfd, buffer, MAX_BUFFER_SIZE, MSG_CONFIRM,
                   (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Subscribing to topic on broker failed!");
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
    /*
     * Options I'd like
     * - server port
     * - server host
     * - verbose
     */
    subscribeToBroker();

    return EXIT_SUCCESS;
}
