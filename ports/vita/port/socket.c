#include <string.h>
#include <stdint.h>
#include <arpa/inet.h>

// inet_ntop implementation for PSVita
const char *inet_ntop(int af, const void *src, char *dst, size_t size) {
    if (af == AF_INET && size >= 16) {
        const struct in_addr *addr = (const struct in_addr *)src;
        uint32_t ip = addr->s_addr;
        sprintf(dst, "%d.%d.%d.%d",
                (ip >> 24) & 0xFF,
                (ip >> 16) & 0xFF,
                (ip >> 8) & 0xFF,
                ip & 0xFF);
        return dst;
    }
    return NULL;
}

// inet_pton implementation for PSVita
int inet_pton(int af, const char *src, void *dst) {
    if (af == AF_INET) {
        struct in_addr *addr = (struct in_addr *)dst;
        int a, b, c, d;
        if (sscanf(src, "%d.%d.%d.%d", &a, &b, &c, &d) == 4) {
            addr->s_addr = (a << 24) | (b << 16) | (c << 8) | d;
            return 1;
        }
    }
    return 0;
}

// Network byte order conversion
uint32_t htonl(uint32_t hostlong) {
    return ((hostlong >> 24) & 0xFF) |
           ((hostlong >> 8) & 0xFF00) |
           ((hostlong << 8) & 0xFF0000) |
           ((hostlong << 24) & 0xFF000000);
}

uint16_t htons(uint16_t hostshort) {
    return ((hostshort >> 8) & 0xFF) | ((hostshort << 8) & 0xFF00);
}

uint32_t ntohl(uint32_t netlong) {
    return htonl(netlong); // Same operation
}

uint16_t ntohs(uint16_t netshort) {
    return htons(netshort); // Same operation
}

// Thread-safe version of strtok
char *strtok_r(char *str, const char *delim, char **saveptr) {
    char *token;
    if (str == NULL) {
        str = *saveptr;
    }
    str += strspn(str, delim);
    if (*str == '\0') {
        *saveptr = str;
        return NULL;
    }
    token = str;
    str = strpbrk(token, delim);
    if (str == NULL) {
        *saveptr = token + strlen(token);
    } else {
        *str = '\0';
        *saveptr = str + 1;
    }
    return token;
}

// Get string length with maximum limit
size_t strnlen(const char *s, size_t maxlen) {
    size_t len = 0;
    while (len < maxlen && s[len] != '\0') {
        len++;
    }
    return len;
}