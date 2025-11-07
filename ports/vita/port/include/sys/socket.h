#ifndef _SYS_SOCKET_H_
#define _SYS_SOCKET_H_

#ifdef __PSVITA__
/* PSVita: use VitaSDK's real socket definitions */
#include <psp2/net/net.h>
/* Include VitaSDK's socket definitions (which #includes our sys/socket.h from VitaSDK) */
/* Let the actual VitaSDK header be included, then add our macros */
#else
/* Non-PSVita: use LwIP socket wrapper */
#include <lwip/sockets.h>
#endif

/* Add missing constants for all platforms */
#define SOCK_SEQPACKET 5
#define IP_ADD_MEMBERSHIP 35
#define IP_DROP_MEMBERSHIP 36

#endif /* _SYS_SOCKET_H_ */