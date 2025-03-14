#ifndef _DNSSERVER_H_
#define _DNSSERVER_H_

#include "lwip/ip_addr.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct dns_server_t_ {
    struct udp_pcb *udp;
    ip_addr_t ip;
} dns_server_t;

void dns_server_init(dns_server_t *d, ip_addr_t *ip);
void dns_server_deinit(dns_server_t *d);

#ifdef __cplusplus
}
#endif

#endif