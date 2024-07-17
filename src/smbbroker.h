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

#endif //AI1027_HAUSARBEIT_03_SMBBROKER_H
