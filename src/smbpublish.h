//
// Created by maximizzar on 17.07.24.
//

#ifndef AI1027_HAUSARBEIT_03_SMBPUBLISH_H
#define AI1027_HAUSARBEIT_03_SMBPUBLISH_H

#include "smbcommon.h"

typedef struct {
    unsigned short port;
    bool verbose;
    int sleep;
} CliOptions;

char *server_hostname[256];

/* Program documentation. */
const char *argp_program_version = "SMB 1.1.0";
const char *argp_program_bug_address = "<mail@maximizzar.de>";
static char doc[] = "SMB: smb-publish\vThe Publisher build messages from Datasource's "
                    "and sends the Data under a Topi to it's Broker.";
static char args_doc[] = "hostname topic";

#endif //AI1027_HAUSARBEIT_03_SMBPUBLISH_H
