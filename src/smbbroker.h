//
// Created by maximizzar on 17.07.24.
//

#ifndef AI1027_HAUSARBEIT_03_SMBBROKER_H
#define AI1027_HAUSARBEIT_03_SMBBROKER_H

#include "smbcommon.h"

#define SUBSCRIBERS_MAX 48
#define CIRCULARBUFFERSIZE 16

typedef struct  {
    char address[INET_ADDRSTRLEN];
    int port;
    char topic[MAX_BUFFER_SIZE / 2 - 8];
} Subscriber;

typedef struct {
    unsigned short port;
    bool verbose;
} CliOptions;

/* Program documentation. */
const char *argp_program_version = "SMB 1.1.0";
const char *argp_program_bug_address = "<mail@maximizzar.de>";
static char doc[] = "SMB: smb-broker\vThe broker receives messages from the publisher and forward them to the subscriber, "
                    "if the subscriber has subscribed to receive the message.";
static char args_doc[] = "";

#endif //AI1027_HAUSARBEIT_03_SMBBROKER_H
