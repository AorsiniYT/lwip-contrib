#include "miniupnpc.h"

// UPnP functions - stub implementations (PSVita doesn't support UPnP)
int upnpDiscoverAll(int delay, const char *multicastif, const char *minissdpdsock, int sameport, int ipv6, int *error) {
    return 0; // No devices found
}

int UPNP_GetValidIGD(struct UPNPDev *devlist, struct UPNPUrls *urls, struct IGDdatas *data, char *lanaddr, int lanaddrlen) {
    return 0; // No valid IGD found
}

void freeUPNPDevlist(struct UPNPDev *devlist) {
    // Nothing to free
}

void FreeUPNPUrls(struct UPNPUrls *urls) {
    // Nothing to free
}

int UPNP_GetExternalIPAddress(const char *controlURL, const char *servicetype, char *extipaddr, char *error) {
    return 0; // Not supported
}

int UPNP_GetSpecificPortMappingEntry(const char *controlURL, const char *servicetype, const char *extport, const char *proto, const char *remotehost, char *intclient, char *intport, char *desc, char *enabled, char *leasetime) {
    return 0; // Not supported
}

int UPNP_AddPortMapping(const char *controlURL, const char *servicetype, const char *extport, const char *inport, const char *inclient, const char *proto, const char *desc, const char *remotehost, const char *leasetime) {
    return 0; // Not supported
}