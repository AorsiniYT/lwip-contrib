#ifndef _MINIUPNPC_H_
#define _MINIUPNPC_H_

// UPnP structures and functions - stub implementations for PSVita
struct UPNPDev {
    struct UPNPDev *pNext;
    char *descURL;
    char *st;
    char *usn;
};

struct UPNPUrls {
    char *controlURL;
    char *ipcondescURL;
    char *controlURL_CIF;
    char *controlURL_6FC;
    char *rootdescURL;
};

struct IGDdatas {
    char cureltname[128];
    char urlbase[128];
    char presentationurl[128];
    char level2forwarding[128];
    char leaseTime[128];
    char connectType[128];
    char lastConnectError[128];
    char relativeTime[128];
    char upnpconn[128];
    char rsipavailable[128];
    char natenabled[128];
    char pcpserver[128];
    char firewallEnabled[128];
    char inboundpinholeallowed[128];
    char outboundpinholeallowed[128];
    char portmappingsallowed[128];
    char portmappingnumberofentries[128];
    char portmappingtablename[128];
    char remotehost[128];
    char externalport[128];
    char protocol[128];
    char internalport[128];
    char internalclient[128];
    char portmappingdescription[128];
    char portmappingenabled[128];
    char portmappingleaseTime[128];
    char portmappingremotehost[128];
};

// Function declarations - stubs
int upnpDiscoverAll(int delay, const char *multicastif, const char *minissdpdsock, int sameport, int ipv6, int *error);
int UPNP_GetValidIGD(struct UPNPDev *devlist, struct UPNPUrls *urls, struct IGDdatas *data, char *lanaddr, int lanaddrlen);
void freeUPNPDevlist(struct UPNPDev *devlist);
void FreeUPNPUrls(struct UPNPUrls *urls);
int UPNP_GetExternalIPAddress(const char *controlURL, const char *servicetype, char *extipaddr, char *error);
int UPNP_GetSpecificPortMappingEntry(const char *controlURL, const char *servicetype, const char *extport, const char *proto, const char *remotehost, char *intclient, char *intport, char *desc, char *enabled, char *leasetime);
int UPNP_AddPortMapping(const char *controlURL, const char *servicetype, const char *extport, const char *inport, const char *inclient, const char *proto, const char *desc, const char *remotehost, const char *leasetime);

#endif /* _MINIUPNPC_H_ */