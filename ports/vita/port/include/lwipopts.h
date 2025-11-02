#ifndef LWIPOPTS_H_
#define LWIPOPTS_H_

// Define FD_SETSIZE early so lwIP uses the same value
// Must be >= MEMP_NUM_NETCONN (1024) to avoid socket allocation errors
#ifndef FD_SETSIZE
#define FD_SETSIZE 2048
#endif

// Include the main lwIP options from libzt
#include "../../../../src/lwipopts.h"

// Define LWIP_PBUF_CUSTOM_DATA if not defined
#ifndef LWIP_PBUF_CUSTOM_DATA
#define LWIP_PBUF_CUSTOM_DATA
#endif

#endif /* LWIPOPTS_H_ */