#ifndef LWIP_ARCH_SYS_ARCH_H
#define LWIP_ARCH_SYS_ARCH_H

#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/clib.h>

#define SCE_UID_INVALID 0xFFFFFFFF

typedef SceUID sys_sem_t;
typedef SceUID sys_mutex_t;
typedef SceUID sys_mbox_t;
typedef SceUID sys_thread_t;

#define SYS_SEM_NULL SCE_UID_INVALID
#define SYS_MUTEX_NULL SCE_UID_INVALID
#define SYS_MBOX_NULL SCE_UID_INVALID

#define sys_sem_valid(sem) (((sem) != SCE_UID_INVALID))
#define sys_sem_set_invalid(sem) do { *(sem) = SCE_UID_INVALID; } while(0)

#define sys_mutex_valid(mutex) (((mutex) != SCE_UID_INVALID))
#define sys_mutex_set_invalid(mutex) do { *(mutex) = SCE_UID_INVALID; } while(0)

#define sys_mbox_valid(mbox) (((mbox) != SCE_UID_INVALID))
#define sys_mbox_set_invalid(mbox) do { *(mbox) = SCE_UID_INVALID; } while(0)

#endif /* LWIP_ARCH_SYS_ARCH_H */