#ifndef _ARPA_INET_H_
#define _ARPA_INET_H_

#include <stdint.h>
#include <stddef.h>

// Use lwIP functions
#define inet_ntop(af, src, dst, size) lwip_inet_ntop(af, src, dst, size)
#define inet_pton(af, src, dst) lwip_inet_pton(af, src, dst)

// Network byte order conversion
#define htonl(hostlong) lwip_htonl(hostlong)
#define htons(hostshort) lwip_htons(hostshort)
#define ntohl(netlong) lwip_ntohl(netlong)
#define ntohs(netshort) lwip_ntohs(netshort)

#endif /* _ARPA_INET_H_ */