//
// Created by maximizzar on 17.07.24.
//

#include "smbpublish.h"
#include "smbcommon.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

void publishMessage(char buffer[MAX_BUFFER_SIZE], Message *message) {
    socket_serialization(buffer, message);

    int sockfd;
    struct sockaddr_in servaddr;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(SERVER_IP);
    servaddr.sin_port = htons(PORT);

    sendto(sockfd, buffer, MAX_BUFFER_SIZE, MSG_CONFIRM,
           (const struct sockaddr *)&servaddr, sizeof(servaddr));

    printf("Message \"%s\" published: on %s at %s\n",
           message->data, message->topic,
           timestamp_to_string(message->unix_timestamp));
    close(sockfd);
}

int main() {
    // Example message to publish
    char buffer[MAX_BUFFER_SIZE];
    Message message;
    message.unix_timestamp = time(NULL);
    strcpy(message.topic, "Hello World");
    strcpy(message.data, "Hi Mom!");
    publishMessage(buffer, &message);
    return EXIT_SUCCESS;
}
