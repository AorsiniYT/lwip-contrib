#include "unistd.h"

// Define FD_SETSIZE BEFORE including lwIP to avoid socket allocation errors
#ifndef FD_SETSIZE
#define FD_SETSIZE 2048
#endif

#include "lwipopts.h"
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/clib.h>
#include <lwip/sockets.h>

int usleep(unsigned int usec) {
    sceKernelDelayThread(usec);
    return 0;
}

// File operations - for sockets, delegate to lwIP
int close(int fd) {
    return lwip_close(fd);
}

ssize_t read(int fd, void *buf, size_t count) {
    return lwip_read(fd, buf, count);
}

ssize_t write(int fd, const void *buf, size_t count) {
    return lwip_write(fd, buf, count);
}

// Process operations
pid_t getpid(void) {
    return 1; // PSVita doesn't have processes like Unix, return dummy value
}

int pipe(int pipefd[2]) {
    // PSVita doesn't support pipes, return error
    return -1;
}