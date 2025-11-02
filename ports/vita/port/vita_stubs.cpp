#include "unistd.h"
#include "strings.h"
#include "ifaddrs.h"

// Define FD_SETSIZE BEFORE including lwIP to avoid socket allocation errors
#ifndef FD_SETSIZE
#define FD_SETSIZE 2048
#endif

#include "lwipopts.h"
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/clib.h>
#include <lwip/sockets.h>

// errno variable for error handling
int errno = 0;

// unistd.h functions
int usleep(unsigned int usec) {
    sceKernelDelayThread(usec);
    return 0;
}

int close(int fd) {
    return lwip_close(fd);
}

ssize_t read(int fd, void *buf, size_t count) {
    return lwip_read(fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return lwip_write(fd, buf, count);
}

pid_t getpid(void) {
    return 1; // PSVita doesn't have processes like Unix, return dummy value
}

int pipe(int pipefd[2]) {
    // PSVita doesn't support pipes, return error
    return -1;
}

// strings.h functions
size_t strnlen(const char *s, size_t maxlen) {
    size_t len = 0;
    while (len < maxlen && s[len] != '\0') {
        len++;
    }
    return len;
}

char *strtok_r(char *str, const char *delim, char **saveptr) {
    if (str == NULL) {
        str = *saveptr;
    }
    
    if (str == NULL || *str == '\0') {
        *saveptr = NULL;
        return NULL;
    }
    
    // Skip leading delimiters
    size_t spanlen = 0;
    while (str[spanlen] != '\0' && strchr(delim, str[spanlen]) != NULL) {
        spanlen++;
    }
    
    if (str[spanlen] == '\0') {
        *saveptr = NULL;
        return NULL;
    }
    
    char *token = str + spanlen;
    
    // Find end of token
    while (str[spanlen] != '\0' && strchr(delim, str[spanlen]) == NULL) {
        spanlen++;
    }
    
    if (str[spanlen] != '\0') {
        str[spanlen] = '\0';
        *saveptr = str + spanlen + 1;
    } else {
        *saveptr = NULL;
    }
    
    return token;
}

// ifaddrs.h functions
int getifaddrs(struct ifaddrs **ifap) {
    *ifap = NULL;
    return 0; // Return 0 (success) with empty list on PSVita
}

void freeifaddrs(struct ifaddrs *ifa) {
    // No-op stub
}

