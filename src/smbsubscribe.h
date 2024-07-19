//
// Created by maximizzar on 17.07.24.
//

#ifndef AI1027_HAUSARBEIT_03_SMBSUBSCRIBE_H
#define AI1027_HAUSARBEIT_03_SMBSUBSCRIBE_H

#include "smbcommon.h"

/* options */
static struct argp_option options[] = {
        {"verbose",  'v', 0,       0, "More Verbose logging"},
        {"port",     'p', "PORT",  0, "Server Port" },
        {"legacy_ip", '4', 0,          0, "Use ipv4 exclusively"},
        { 0 },
};

/* Program documentation. */
const char *argp_program_version = "SMB 1.1.0";
const char *argp_program_bug_address = "<mail@maximizzar.de>";
static char doc[] = "SMB: smb-broker\vThe Subscriber receives messages from it's subscribed Topic(s).";
static char args_doc[] = "hostname topic";

typedef struct {
    unsigned short port, legacy_ip;
    bool verbose;
} CliOptions;

/*
 * Global vars
 */
int sock_fd;
struct addrinfo hints = {0};
char *server_hostname[MAX_BUFFER_SIZE / 8];
char topic[MAX_BUFFER_SIZE];

#endif //AI1027_HAUSARBEIT_03_SMBSUBSCRIBE_H
