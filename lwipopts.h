#ifndef _LWIPOPTS_H
#define _LWIPOPTS_H

// Generally you would define your own explicit list of lwIP options
// (see https://www.nongnu.org/lwip/2_1_x/group__lwip__opts.html)
//
// This example uses a common include to avoid repetition

#define HTTPD_USE_CUSTOM_FSDATA 1
#include "lwipopts_httpd.h"
#define LWIP_MDNS_RESPONDER 1
#define LWIP_NUM_NETIF_CLIENT_DATA 2
#define MEMP_NUM_UDP_PCB 6
#define MEMP_NUM_TCP_PCB        10
#define MDNS_MAX_SERVICES 1
#define MEMP_NUM_TCP_PCB_LISTEN 6
#define MEMP_NUM_TCP_SEG        TCP_SND_QUEUELEN
#define MEMP_NUM_SYS_TIMEOUT    11

#endif
