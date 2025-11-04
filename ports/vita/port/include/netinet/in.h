#ifndef _NETINET_IN_H_
#define _NETINET_IN_H_

#ifndef __PSVITA__
/* Non-PSVita: Use lwIP */
#include <lwip/inet.h>
#include <sys/socket.h>
#else
/* PSVita: Use VitaSDK directly */
#include <sys/socket.h>
#include <psp2/net/net.h>  /* VitaSDK network types */
#endif

#ifndef __PSVITA__
/* lwIP-specific definitions */
#include <psp2common/net.h>  /* For PSVita network constants */

// Define sa_family_t if not already defined
#ifndef sa_family_t
typedef uint8_t sa_family_t;
#endif

// Define in_port_t if not already defined
#ifndef in_port_t
typedef uint16_t in_port_t;
#endif

// Multicast request structure
struct ip_mreqn {
    struct in_addr imr_multiaddr;
    struct in_addr imr_address;
    int imr_ifindex;
};

// IPv4 multicast options
#ifndef IP_MULTICAST_TTL
#define IP_MULTICAST_TTL        SCE_NET_IP_MULTICAST_TTL
#endif

#ifndef IP_MULTICAST_IF
#define IP_MULTICAST_IF         SCE_NET_IP_MULTICAST_IF
#endif

#ifndef IP_MULTICAST_LOOP
#define IP_MULTICAST_LOOP       SCE_NET_IP_MULTICAST_LOOP
#endif

#if !LWIP_IPV6
// Dummy definition for sockaddr_in6 for IPv4-only builds
struct sockaddr_in6 {
    sa_family_t sin6_family;
    in_port_t sin6_port;
    uint32_t sin6_flowinfo;
    struct in6_addr sin6_addr;
    uint32_t sin6_scope_id;
};
#endif

#endif /* __PSVITA__ */

#endif /* _NETINET_IN_H_ */