//
// Created by maximizzar on 17.07.24.
//

#ifndef AI1027_HAUSARBEIT_03_SMBPUBLISH_H
#define AI1027_HAUSARBEIT_03_SMBPUBLISH_H

#include "smbcommon.h"

/* options */
static struct argp_option options[] = {
        {"verbose",   'v', 0,          0, "More Verbose logging"},
        {"port",      'p', "1-65535",  0, "Server Port" },
        {"sleep",     's', "num",      0, "Time to wait till next measurement"},
        {"legacy_ip", '4', 0,          0, "Use ipv4 exclusively"},
        { 0 },
};

/* Program documentation. */
const char *argp_program_version = "SMB 1.1.0";
const char *argp_program_bug_address = "<mail@maximizzar.de>";
static char doc[] = "SMB: smb-publish\vThe Publisher build messages from Datasource's "
                    "and sends the Data under a Topi to it's Broker.";
static char args_doc[] = "hostname topic";

typedef struct {
    unsigned short port;
    bool verbose, legacy_ip;
    long sleep;
} CliOptions;

/*
 * Global vars
 */
int sock_fd;
struct addrinfo hints = {0};
char *server_hostname[MAX_BUFFER_SIZE / 8];
char topic[MAX_BUFFER_SIZE];

#endif //AI1027_HAUSARBEIT_03_SMBPUBLISH_H
