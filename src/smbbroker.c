//
// Created by maximizzar on 17.07.24.
//

#include "smbbroker.h"
#include "main.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_CLIENTS 5
#define MAX_BUFFER_SIZE 1024

struct Subscriber {
    char address[INET_ADDRSTRLEN];
    int port;
};

int main() {
    int sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[MAX_BUFFER_SIZE];
    char *hello = "Hello from server";
    struct Subscriber subscribers[MAX_CLIENTS];
    int num_subscribers = 0;

    // Create UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind socket to port
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    int len, n;
    len = sizeof(cliaddr);

    while(1) {
        n = recvfrom(sockfd, (char *)buffer, MAX_BUFFER_SIZE, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
        buffer[n] = '\0';
        printf("Client message: %s\n", buffer);

        // Check if subscriber is new
        int new_subscriber = 1;
        for (int i = 0; i < num_subscribers; i++) {
            if (strcmp(inet_ntoa(cliaddr.sin_addr), subscribers[i].address) == 0 && ntohs(cliaddr.sin_port) == subscribers[i].port) {
                new_subscriber = 0;
                break;
            }
        }

        if (new_subscriber) {
            // Add new subscriber
            if (num_subscribers < MAX_CLIENTS) {
                strcpy(subscribers[num_subscribers].address, inet_ntoa(cliaddr.sin_addr));
                subscribers[num_subscribers].port = ntohs(cliaddr.sin_port);
                num_subscribers++;
                printf("New subscriber added: %s:%d\n", subscribers[num_subscribers-1].address, subscribers[num_subscribers-1].port);
            } else {
                printf("Max subscribers reached. Cannot add new subscriber.\n");
            }
        }

        // Send response to subscriber
        sendto(sockfd, (const char *)hello, strlen(hello), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);
        printf("Hello message sent\n");
    }

    return 0;
}