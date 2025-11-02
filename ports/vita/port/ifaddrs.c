// Define FD_SETSIZE BEFORE including any socket headers
#ifndef FD_SETSIZE
#define FD_SETSIZE 2048
#endif

#include "ifaddrs.h"
#include <stdlib.h>
#include <string.h>

// Basic implementation for PSVita - returns empty list
int getifaddrs(struct ifaddrs **ifap) {
    *ifap = NULL; // No interfaces to report
    return 0;
}

void freeifaddrs(struct ifaddrs *ifa) {
    // Nothing to free
}