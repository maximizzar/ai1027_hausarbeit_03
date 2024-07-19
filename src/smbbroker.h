//
// Created by maximizzar on 17.07.24.
//

#ifndef AI1027_HAUSARBEIT_03_SMBBROKER_H
#define AI1027_HAUSARBEIT_03_SMBBROKER_H

#include "smbcommon.h"

#define SUBSCRIBERS_MAX 48

/* options */
static struct argp_option options[] = {
        {"verbose",  'v', 0,          0, "More Verbose logging"},
        {"port",     'p', "1-65535",  0, "Server Port" },
        { 0 },
};

/* Program documentation. */
const char *argp_program_version = "SMB 1.1.0";
const char *argp_program_bug_address = "<mail@maximizzar.de>";
static char doc[] = "SMB: smb-broker\vThe broker receives messages from the publisher and forward them to the subscriber, "
                    "if the subscriber has subscribed to receive the message.";
static char args_doc[] = "";

typedef struct {
    unsigned short port;
    bool verbose;
} CliOptions;

typedef struct  {
    struct sockaddr_in6 addr;
    char address[INET6_ADDRSTRLEN];
    int port;
    char topic[MAX_BUFFER_SIZE / 2 - 8];
} Subscriber;

/*
 * Global vars
 */
int sockfd;

#endif //AI1027_HAUSARBEIT_03_SMBBROKER_H
