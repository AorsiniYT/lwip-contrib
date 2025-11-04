#ifndef _SYS_SOCKET_H_
#define _SYS_SOCKET_H_

#ifndef __PSVITA__
#include <lwip/sockets.h>
#else
/* PSVita uses VitaSDK sockets directly */
#include <psp2/net/net.h>
#endif

// Add missing constants
#define AF_UNIX 1
#define PF_INET6 AF_INET6
#define SOCK_SEQPACKET 5
#define IP_ADD_MEMBERSHIP 35
#define IP_DROP_MEMBERSHIP 36

#ifndef __PSVITA__
// Typedef for ZeroTier sockaddr (forward declared)
typedef struct sockaddr ZeroTier_sockaddr;

// Function defines with casts
// #define bind(sockfd, addr, addrlen) lwip_bind(sockfd, (const struct sockaddr*)(addr), addrlen)
inline int bind(int sockfd, const void *addr, socklen_t addrlen) {
    return lwip_bind(sockfd, (const struct sockaddr*)addr, addrlen);
}
#define connect(sockfd, addr, addrlen) lwip_connect(sockfd, (const struct sockaddr*)(const void*)(addr), addrlen)
#define accept(sockfd, addr, addrlen) lwip_accept(sockfd, (struct sockaddr*)(void*)(addr), addrlen)
#define sendto(sockfd, buf, len, flags, dest_addr, addrlen) lwip_sendto(sockfd, buf, len, flags, (const struct sockaddr*)(const void*)(dest_addr), addrlen)
#define recvfrom(sockfd, buf, len, flags, src_addr, addrlen) lwip_recvfrom(sockfd, buf, len, flags, (struct sockaddr*)(void*)(src_addr), addrlen)
#define getpeername(sockfd, addr, addrlen) lwip_getpeername(sockfd, (struct sockaddr*)(void*)(addr), addrlen)
#define getsockname(sockfd, addr, addrlen) lwip_getsockname(sockfd, (struct sockaddr*)(void*)(addr), addrlen)

// Use macros only for non-conflicting functions
#define socket(domain, type, protocol) lwip_socket(domain, type, protocol)
#define setsockopt(sockfd, level, optname, optval, optlen) lwip_setsockopt(sockfd, level, optname, optval, optlen)
#define getsockopt(sockfd, level, optname, optval, optlen) lwip_getsockopt(sockfd, level, optname, optval, optlen)
#define listen(sockfd, backlog) lwip_listen(sockfd, backlog)
#define ioctl(fd, request, argp) lwip_ioctl(fd, request, argp)
#ifdef __cplusplus
#define select(nfds, readfds, writefds, exceptfds, timeout) lwip_select(nfds, readfds, writefds, exceptfds, reinterpret_cast<struct timeval*>(timeout))
#else
#define select(nfds, readfds, writefds, exceptfds, timeout) lwip_select(nfds, readfds, writefds, exceptfds, (struct timeval*)(timeout))
#endif

// Inline functions for potentially conflicting names
inline ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
    return lwip_send(sockfd, buf, len, flags);
}
inline ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
    return lwip_recv(sockfd, buf, len, flags);
}
#else
/* PSVita uses VitaSDK socket functions directly (mapped via socket_psvita.h macros) */
typedef struct sockaddr ZeroTier_sockaddr;

#define socket(domain, type, protocol) sceNetSocket(NULL, domain, type, protocol)
#define bind(sockfd, addr, addrlen) sceNetBind(sockfd, (SceNetSockaddr*)(addr), addrlen)
#define connect(sockfd, addr, addrlen) sceNetConnect(sockfd, (SceNetSockaddr*)(addr), addrlen)
#define accept(sockfd, addr, addrlen) sceNetAccept(sockfd, (SceNetSockaddr*)(addr), addrlen)
#define listen(sockfd, backlog) sceNetListen(sockfd, backlog)
#define send(sockfd, buf, len, flags) sceNetSend(sockfd, buf, len, flags)
#define recv(sockfd, buf, len, flags) sceNetRecv(sockfd, buf, len, flags)
#define sendto(sockfd, buf, len, flags, dest_addr, addrlen) sceNetSendto(sockfd, buf, len, flags, (SceNetSockaddr*)(dest_addr), addrlen)
#define recvfrom(sockfd, buf, len, flags, src_addr, addrlen) sceNetRecvfrom(sockfd, buf, len, flags, (SceNetSockaddr*)(src_addr), addrlen)
#define setsockopt(sockfd, level, optname, optval, optlen) sceNetSetsockopt(sockfd, level, optname, optval, optlen)
#define getsockopt(sockfd, level, optname, optval, optlen) sceNetGetsockopt(sockfd, level, optname, optval, optlen)
#define getsockname(sockfd, addr, addrlen) sceNetGetsockname(sockfd, (SceNetSockaddr*)(addr), addrlen)
#define getpeername(sockfd, addr, addrlen) sceNetGetpeername(sockfd, (SceNetSockaddr*)(addr), addrlen)
#define select(nfds, readfds, writefds, exceptfds, timeout) (-1)
#define ioctl(fd, request, argp) (-1)
#endif

// FD macros
#ifndef FD_SET
#define FD_SET(fd, set) do { \
    if (((set)->__fds_bits[(fd) / 8] & (1 << ((fd) % 8))) == 0) { \
        (set)->__fds_bits[(fd) / 8] |= (1 << ((fd) % 8)); \
    } \
} while(0)
#endif

#ifndef FD_CLR
#define FD_CLR(fd, set) do { \
    (set)->__fds_bits[(fd) / 8] &= ~(1 << ((fd) % 8)); \
} while(0)
#endif

#ifndef FD_ISSET
#define FD_ISSET(fd, set) (((set)->__fds_bits[(fd) / 8] & (1 << ((fd) % 8))) != 0)
#endif

#ifndef FD_ZERO
#define FD_ZERO(set) memset((set), 0, sizeof(fd_set))
#endif

// IPv6 constants (defined even when LWIP_IPV6=0 for compatibility)
#define IPPROTO_IPV6 41
#define IPV6_MULTICAST_HOPS 10

#endif /* _SYS_SOCKET_H_ */