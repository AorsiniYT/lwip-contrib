#include <psp2/kernel/threadmgr.h>

// Define pthread types for compatibility
typedef SceUID pthread_t;
typedef void pthread_attr_t;

// Thread functions - basic implementations using PSVita threads
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void*), void *arg) {
    // Create a PSVita thread
    SceUID thid = sceKernelCreateThread("zt_thread", (SceKernelThreadEntry)start_routine, 0x10000100, 0x10000, 0, 0, NULL);
    if (thid < 0) return -1;
    *thread = thid;
    sceKernelStartThread(thid, 0, arg);
    return 0;
}

int pthread_join(pthread_t thread, void **retval) {
    sceKernelWaitThreadEnd(thread, NULL, NULL);
    return 0;
}

void pthread_exit(void *retval) {
    sceKernelExitThread(0);
}

// Thread attribute functions - stubs
int pthread_attr_init(pthread_attr_t *attr) {
    return 0;
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize) {
    return 0;
}

int pthread_attr_destroy(pthread_attr_t *attr) {
    return 0;
}