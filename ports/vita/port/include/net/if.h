#ifndef _NET_IF_H_
#define _NET_IF_H_

#include <stdint.h>
#include <sys/socket.h>

#define IFNAMSIZ 16

struct if_nameindex {
    unsigned int if_index;
    char *if_name;
};

#ifndef LWIP_SOCKET
struct ifreq {
    char ifr_name[IFNAMSIZ];
    union {
        struct sockaddr ifru_addr;
        struct sockaddr ifru_dstaddr;
        struct sockaddr ifru_broadaddr;
        struct sockaddr ifru_netmask;
        struct sockaddr ifru_hwaddr;
        short ifru_flags;
        int ifru_ivalue;
        int ifru_mtu;
        char ifru_slave[IFNAMSIZ];
        char ifru_newname[IFNAMSIZ];
        void *ifru_data;
    } ifr_ifru;
};

#define ifr_addr        ifr_ifru.ifru_addr
#define ifr_dstaddr     ifr_ifru.ifru_dstaddr
#define ifr_broadaddr   ifr_ifru.ifru_broadaddr
#define ifr_netmask     ifr_ifru.ifru_netmask
#define ifr_hwaddr      ifr_ifru.ifru_hwaddr
#define ifr_flags       ifr_ifru.ifru_flags
#define ifr_ivalue      ifr_ifru.ifru_ivalue
#define ifr_mtu         ifr_ifru.ifru_mtu
#define ifr_slave       ifr_ifru.ifru_slave
#define ifr_newname     ifr_ifru.ifru_newname
#define ifr_data        ifr_ifru.ifru_data
#endif

// Interface flags
#define IFF_UP          0x1
#define IFF_BROADCAST   0x2
#define IFF_DEBUG       0x4
#define IFF_LOOPBACK    0x8
#define IFF_POINTOPOINT 0x10
#define IFF_NOTRAILERS  0x20
#define IFF_RUNNING     0x40
#define IFF_NOARP       0x80
#define IFF_PROMISC     0x100
#define IFF_ALLMULTI    0x200
#define IFF_MASTER      0x400
#define IFF_SLAVE       0x800
#define IFF_MULTICAST   0x1000
#define IFF_PORTSEL     0x2000
#define IFF_AUTOMEDIA   0x4000
#define IFF_DYNAMIC     0x8000

// Ioctl commands
#define SIOCGIFFLAGS    0x8913
#define SIOCSIFFLAGS    0x8914
#define SIOCGIFADDR     0x8915
#define SIOCSIFADDR     0x8916
#define SIOCGIFDSTADDR  0x8917
#define SIOCSIFDSTADDR  0x8918
#define SIOCGIFBRDADDR  0x8919
#define SIOCSIFBRDADDR  0x891a
#define SIOCGIFNETMASK  0x891b
#define SIOCSIFNETMASK  0x891c
#define SIOCGIFMETRIC   0x891d
#define SIOCSIFMETRIC   0x891e
#define SIOCGIFMTU      0x8921
#define SIOCSIFMTU      0x8922
#define SIOCGIFHWADDR   0x8927
#define SIOCSIFHWADDR   0x8924
#define SIOCGIFNAME     0x8910

#endif /* _NET_IF_H_ */