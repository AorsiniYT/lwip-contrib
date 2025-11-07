#ifndef LWIP_ARCH_CC_H
#define LWIP_ARCH_CC_H

// Include lwIP options first
#include "lwipopts.h"

// Define lwIP macros that must be available before including lwIP headers
#ifndef LWIP_PBUF_CUSTOM_DATA
#define LWIP_PBUF_CUSTOM_DATA
#endif

#ifndef LWIP_PBUF_REF_T
#define LWIP_PBUF_REF_T u8_t
#endif

#include <stddef.h>
#include <stdint.h>
#include <psp2/kernel/clib.h>  // For sceClibPrintf

// Define PSVita platform
#define LWIP_UNIX_VITA

// Basic types for lwIP
typedef uint8_t u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef int8_t s8_t;
typedef int16_t s16_t;
typedef int32_t s32_t;

// Define pid_t for unistd.h
typedef int pid_t;

#define LWIP_TIMEVAL_PRIVATE 0
#include <sys/time.h>

#define LWIP_ERRNO_INCLUDE <errno.h>

// PSVita socket headers are included as needed, not here to avoid inclusion order issues

#define LWIP_RAND() ((u32_t)rand())

/* different handling for unit test, normally not needed */
#ifdef LWIP_NOASSERT_ON_ERROR
#define LWIP_ERROR(message, expression, handler) do { if (!(expression)) { \
  handler;}} while(0)
#endif

// Comment out the following line to use lwIP's default diagnostic printing routine
#define LWIP_PLATFORM_DIAG(x)    do {sceClibPrintf x;} while(0)

struct sio_status_s;
typedef struct sio_status_s sio_status_t;
#define sio_fd_t sio_status_t*
#define __sio_fd_t_defined

typedef unsigned int sys_prot_t;

#endif /* LWIP_ARCH_CC_H */