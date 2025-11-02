/*
 * Additional stubs for miniupnpc and lwIP compatibility on PSVita
 */

#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* Stub for if_nametoindex - not available on PSVita */
unsigned int if_nametoindex(const char *ifname) {
    if (ifname == NULL) return 0;
    /* Return a dummy interface index */
    /* In a real implementation, you would map the interface name to an actual index */
    return 1;
}

/* Stub for getnameinfo - for hostname resolution */
int getnameinfo(const struct sockaddr *sa, socklen_t salen,
                char *host, socklen_t hostlen,
                char *serv, socklen_t servlen, int flags) {
    /* Minimal stub - just return an error */
    /* PSVita probably doesn't support this properly anyway */
    if (host && hostlen > 0) {
        host[0] = '\0';
    }
    if (serv && servlen > 0) {
        serv[0] = '\0';
    }
    return -1;  /* Return error */
}

/* Provide weak definition of in6addr_any for compatibility */
/* This will be overridden by lwIP's definition if available */
__attribute__((weak))
const struct in6_addr in6addr_any = {{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}};


