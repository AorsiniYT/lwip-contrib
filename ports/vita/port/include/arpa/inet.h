#ifndef _ARPA_INET_H_
#define _ARPA_INET_H_

/* This file is deprecated - use <lwip/def.h> and <lwip/ip_addr.h> instead
   which provide all the necessary macros and functions.
   We only provide inet_ntop and inet_pton wrappers here.
*/

#include <lwip/def.h>
#include <lwip/ip_addr.h>

#ifdef __cplusplus
extern "C" {
#endif

/* lwIP already provides htonl, htons, ntohl, ntohs in lwip/def.h */

/* Provide inet_ntop and inet_pton if not already available */
#ifndef HAVE_INET_NTOP
const char *inet_ntop(int af, const void *src, char *dst, int cnt);
#endif

#ifndef HAVE_INET_PTON
int inet_pton(int af, const char *src, void *dst);
#endif

#ifdef __cplusplus
}
#endif

#endif /* _ARPA_INET_H_ */