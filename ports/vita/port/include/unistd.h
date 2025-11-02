#ifndef _UNISTD_H_
#define _UNISTD_H_

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Define ssize_t for POSIX compatibility
typedef int ssize_t;

// Define pid_t if not already defined
#ifndef pid_t
typedef int pid_t;
#endif

// Sleep for microseconds
int usleep(unsigned int usec);

// File operations
int close(int fd);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);

// Process operations
pid_t getpid(void);
int pipe(int pipefd[2]);

#ifdef __cplusplus
}
#endif

#endif /* _UNISTD_H_ */