//
// Created by maximizzar on 17.07.24.
//

#include "smbpublish.h"
#include "smbcommon.h"

void publishMessage(char buffer[MAX_BUFFER_SIZE], Message *message) {
    socket_serialization(buffer, message);

    int sockfd;
    struct sockaddr_in servaddr = {0};

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    servaddr.sin_port = htons(PORT);

    sendto(sockfd, buffer, MAX_BUFFER_SIZE, MSG_CONFIRM,
           (const struct sockaddr *)&servaddr, sizeof(servaddr));

    printf("Published at %s on Topic %s => %s\n",
           timestamp_to_string(message->unix_timestamp),
           message->topic, message->data);
    close(sockfd);
}

int main() {
    /*
     * Options I'd like
     * - sleep 1 - 60
     * - server port
     * - server host
     * - verbose
     */
    // Example message to publish
    char buffer[MAX_BUFFER_SIZE];
    Message message;
    message.unix_timestamp = time(NULL);
    strcpy(message.topic, "Hello World");
    strcpy(message.data, "Hi, Mom!");
    publishMessage(buffer, &message);
    return EXIT_SUCCESS;
}
