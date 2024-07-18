//
// Created by maximizzar on 17.07.24.
//

#ifndef AI1027_HAUSARBEIT_03_SMBSUBSCRIBE_H
#define AI1027_HAUSARBEIT_03_SMBSUBSCRIBE_H

#include "smbcommon.h"

typedef struct {
    unsigned short port;
    bool verbose;
} CliOptions;

char *server_hostname[256];

/* Program documentation. */
const char *argp_program_version = "SMB 1.1.0";
const char *argp_program_bug_address = "<mail@maximizzar.de>";
static char doc[] = "SMB: smb-broker\vThe Subscriber receives messages from it's subscribed Topic(s).";
static char args_doc[] = "hostname topic";

#endif //AI1027_HAUSARBEIT_03_SMBSUBSCRIBE_H
