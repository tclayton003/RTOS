/* Copyright (c) 2013-2017 GomSpace A/S. All rights reserved. */

#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>

static inline const char * ip_addr_to_string(const void * vsa)
{
    static char return_string[100];

    char tmp_ip[100];
    const struct sockaddr *sa = vsa;
    switch(sa->sa_family) {
        case AF_INET:
            {
                const struct sockaddr_in * in = vsa;
                inet_ntop(AF_INET, &in->sin_addr, tmp_ip, sizeof(tmp_ip));
                snprintf(return_string, sizeof(return_string), "%s:%d", tmp_ip, ntohs(in->sin_port));
            }
            break;

        case AF_INET6:
            {
                const struct sockaddr_in6 * in = vsa;
                inet_ntop(AF_INET6, &in->sin6_addr, tmp_ip, sizeof(tmp_ip));
                snprintf(return_string, sizeof(return_string), "%s:%d", tmp_ip, ntohs(in->sin6_port));
            }
            break;

        default:
            strcpy(return_string, "Unknown AF");
            break;
    }

    return return_string;
}
