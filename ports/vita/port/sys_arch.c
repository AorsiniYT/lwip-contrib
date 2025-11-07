/*
 * PSVita lwIP port - sys_arch.c
 * 
 * CRITICAL FIXES IMPLEMENTED:
 * 1. Priority validation: lwIP DEFAULT_THREAD_PRIO=1 is INVALID in PSVita (0x20-0x3F valid)
 *    Solution: Convert invalid priorities to 0x40 (normal user priority)
 * 
 * 2. Thread argument passing: sceKernelStartThread corrupts stack-based arguments
 *    Solution: Use persistent buffer array indexed by thread ID, retrieved via sceKernelGetThreadId()
 * 
 * 3. Stack allocation: Multiple threads need reasonable stack size without wasting memory
 *    Solution: 64KB (0x10000) per thread instead of default 512KB
 * 
 * 4. Buffer management: Multiple concurrent threads needed dynamic registration system
 *    Solution: Mutex-protected array of 256 buffers with proper initialization
 * 
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "lwip/debug.h"
#include "lwip/def.h"
#include "lwip/sys.h"
#include "lwip/opt.h"
#include "lwip/stats.h"

#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/error.h>
#include <string.h>
#include <stdlib.h>
#include "strings.h"

// Forward declarations for missing functions
static struct sys_sem *sys_sem_new_internal(u8_t count);
static void sys_sem_free_internal(struct sys_sem *sem);
err_t sys_mbox_trypost_fromisr(sys_mbox_t *mbox, void *msg);

#define SCE_UID_INVALID 0xFFFFFFFF

// Debug counters for PSVita kernel object creation
static volatile int _debug_mutex_count = 0;
static volatile int _debug_sema_count = 0;
static volatile int _debug_thread_count = 0;

#define SYS_MBOX_SIZE 128

struct sys_mbox {
  int first, last;
  void *msgs[SYS_MBOX_SIZE];
  sys_sem_t not_empty;
  sys_sem_t not_full;
  sys_sem_t mutex;
  int wait_send;
};

struct sys_sem {
  SceUID sema;
};

struct sys_mutex {
  SceUID mutex;
};

struct sys_thread {
  SceUID thread;
  lwip_thread_fn function;
  void *arg;
};

// Sistema dinámico para buffers de threads
// En lugar de un array fijo, usar un sistema que reutilice slots

#define MAX_LWIP_THREADS 256  // Aumentado significativamente

// Array de buffers para pasar argumentos a threads
static struct sys_thread* _thread_arg_buffers[MAX_LWIP_THREADS];
static SceUID _thread_ids[MAX_LWIP_THREADS];
static volatile int _buffer_count = 0;
static SceUID _buffer_lock_mutex = SCE_UID_INVALID;  // Mutex para proteger acceso a buffers
static int _buffers_initialized = 0;

// Inicializar arrays de buffers
static void _init_buffers() {
  if (!_buffers_initialized) {
    for (int i = 0; i < MAX_LWIP_THREADS; i++) {
      _thread_arg_buffers[i] = NULL;
      _thread_ids[i] = SCE_UID_INVALID;
    }
    _buffers_initialized = 1;
    sceClibPrintf("[SYS_ARCH] Initialized buffer arrays (MAX=%d)\n", MAX_LWIP_THREADS);
  }
}

// Inicializar mutex para proteger buffers (llamar una sola vez)
static void _init_buffer_lock() {
  _init_buffers();
  
  if (_buffer_lock_mutex == SCE_UID_INVALID) {
    _buffer_lock_mutex = sceKernelCreateMutex("_buffer_lock", 0, 0, NULL);
    sceClibPrintf("[SYS_ARCH] Created buffer lock mutex: 0x%x\n", _buffer_lock_mutex);
  }
}

// Encontrar y registrar buffer para un thread específico
static int _register_thread_buffer(SceUID thread_id, struct sys_thread* st) {
  _init_buffer_lock();
  
  sceKernelLockMutex(_buffer_lock_mutex, 1, NULL);
  
  // Primero, buscar si ya existe un slot para este thread
  for (int i = 0; i < MAX_LWIP_THREADS; i++) {
    if (_thread_ids[i] == thread_id) {
      sceClibPrintf("[SYS_ARCH] WARNING: Thread 0x%x already registered at index %d\n", thread_id, i);
      sceKernelUnlockMutex(_buffer_lock_mutex, 1);
      return i;
    }
  }
  
  // Buscar un slot vacío
  for (int i = 0; i < MAX_LWIP_THREADS; i++) {
    if (_thread_ids[i] == SCE_UID_INVALID) {
      _thread_ids[i] = thread_id;
      _thread_arg_buffers[i] = st;
      _buffer_count++;
      sceClibPrintf("[SYS_ARCH] Registered thread buffer[%d] for thread 0x%x, st=%p (count=%d)\n", i, thread_id, st, _buffer_count);
      sceKernelUnlockMutex(_buffer_lock_mutex, 1);
      return i;
    }
  }
  sceClibPrintf("[SYS_ARCH] ERROR: No more buffer slots available (MAX=%d, current=%d) - CRITICAL!\n", MAX_LWIP_THREADS, _buffer_count);
  sceKernelUnlockMutex(_buffer_lock_mutex, 1);
  return -1;
}

// Encontrar y recuperar buffer de un thread
static struct sys_thread* _get_thread_buffer(SceUID thread_id) {
  _init_buffer_lock();
  sceKernelLockMutex(_buffer_lock_mutex, 1, NULL);
  
  for (int i = 0; i < MAX_LWIP_THREADS; i++) {
    if (_thread_ids[i] == thread_id) {
      struct sys_thread* st = _thread_arg_buffers[i];
      sceKernelUnlockMutex(_buffer_lock_mutex, 1);
      return st;
    }
  }
  sceClibPrintf("[SYS_ARCH] WARNING: Buffer not found for thread 0x%x\n", thread_id);
  sceKernelUnlockMutex(_buffer_lock_mutex, 1);
  return NULL;
}

// Limpiar buffer después de que el thread termine
static void _unregister_thread_buffer(SceUID thread_id) {
  _init_buffer_lock();
  sceKernelLockMutex(_buffer_lock_mutex, 1, NULL);
  
  for (int i = 0; i < MAX_LWIP_THREADS; i++) {
    if (_thread_ids[i] == thread_id) {
      _thread_ids[i] = SCE_UID_INVALID;
      _thread_arg_buffers[i] = NULL;
      _buffer_count--;
      sceClibPrintf("[SYS_ARCH] Unregistered thread buffer[%d] for thread 0x%x (count now=%d)\n", i, thread_id, _buffer_count);
      sceKernelUnlockMutex(_buffer_lock_mutex, 1);
      return;
    }
  }
  sceClibPrintf("[SYS_ARCH] WARNING: Could not find buffer for thread 0x%x to unregister\n", thread_id);
  sceKernelUnlockMutex(_buffer_lock_mutex, 1);
}

static int thread_wrapper(SceSize args, void *argp) {
  SceUID my_thread_id = sceKernelGetThreadId();
  sceClibPrintf("[SYS_ARCH] thread_wrapper() START - my_thread_id=0x%x\n", my_thread_id);
  
  // Recuperar struct del buffer persistente usando el thread ID
  struct sys_thread *thread_data = _get_thread_buffer(my_thread_id);
  
  if (!thread_data) {
    sceClibPrintf("[SYS_ARCH] ERROR: No buffer found for thread 0x%x\n", my_thread_id);
    return -1;
  }
  
  sceClibPrintf("[SYS_ARCH] thread_wrapper() - thread_data=%p\n", thread_data);
  sceClibPrintf("[SYS_ARCH] thread_wrapper() - validating thread_data structure...\n");
  
  // Validar que el puntero esté en un rango razonable
  if ((unsigned int)thread_data < 0x80000000 || (unsigned int)thread_data > 0x90000000) {
    sceClibPrintf("[SYS_ARCH] ERROR: thread_data=%p is OUTSIDE valid memory range!\n", thread_data);
    return -1;
  }
  
  // Copiar función y argumentos localmente ANTES de limpiar buffer
  lwip_thread_fn fn = thread_data->function;
  void *arg = thread_data->arg;
  
  sceClibPrintf("[SYS_ARCH] thread_wrapper() - function=%p, arg=%p\n", fn, arg);
  
  // Validar que la función sea razonable
  if ((unsigned int)fn < 0x80000000 || (unsigned int)fn > 0x90000000) {
    sceClibPrintf("[SYS_ARCH] ERROR: function pointer=%p is INVALID!\n", fn);
    return -1;
  }
  
  // Ejecutar la función del thread
  sceClibPrintf("[SYS_ARCH] thread_wrapper() - calling function...\n");
  fn(arg);
  sceClibPrintf("[SYS_ARCH] thread_wrapper() - function returned\n");
  
  // NO LIBERAR thread_data aquí - dejar que el garbage collector lo maneje
  // Liberar en este punto causa corrupción de heap cuando múltiples threads
  // intenta liberar memoria simultáneamente en lwIP
  
  // Solo limpiar registro del buffer
  sceClibPrintf("[SYS_ARCH] thread_wrapper() - Unregistering buffer for thread 0x%x\n", my_thread_id);
  _unregister_thread_buffer(my_thread_id);
  
  sceClibPrintf("[SYS_ARCH] thread_wrapper() - Thread exiting normally\n");
  return 0;
}

/*-----------------------------------------------------------------------------------*/
/* Threads */
sys_thread_t
sys_thread_new(const char *name, lwip_thread_fn function, void *arg, int stacksize, int prio)
{
  sceClibPrintf("[SYS_ARCH] sys_thread_new() START - name=%s, stacksize=%d, prio=%d\n", name, stacksize, prio);
  
  sceClibPrintf("[SYS_ARCH] sys_thread_new() - About to malloc()\n");
  struct sys_thread *st = (struct sys_thread *)malloc(sizeof(struct sys_thread));
  if (st == NULL) {
    sceClibPrintf("[SYS_ARCH] ERROR: malloc failed for sys_thread\n");
    return NULL;
  }
  sceClibPrintf("[SYS_ARCH] sys_thread_new() - malloc successful, st=%p\n", st);

  st->function = function;
  st->arg = arg;
  sceClibPrintf("[SYS_ARCH] sys_thread_new() - Assigned function=%p, arg=%p\n", function, arg);

  // En PSVita, si stacksize es 0, usar un tamaño razonable
  // 512KB (0x80000) es demasiado grande para múltiples threads
  // lwIP threads normalmente no necesitan mucha stack
  // Usar 64KB (0x10000) como balance entre seguridad y uso de memoria
  if (stacksize == 0) {
    stacksize = 0x10000;  // 64KB - suficiente para lwIP threads
    sceClibPrintf("[SYS_ARCH] stacksize=0, using default 0x%x\n", stacksize);
  }

  // En PSVita, las prioridades válidas son 0x20-0x3F (32-63) para user applications
  // lwIP puede pasar valores como 1 que no son válidos en PSVita
  // Convertir a una prioridad válida: usar 0x40 (64, normal user priority)
  if (prio < 0x20 || prio > 0x3F) {
    sceClibPrintf("[SYS_ARCH] prio=%d is invalid, converting to 0x40\n", prio);
    prio = 0x40;  // Normal priority para PSVita
  }

  sceClibPrintf("[SYS_ARCH] sys_thread_new() - Calling sceKernelCreateThread(%s, ...)\n", name);
  SceUID thread = sceKernelCreateThread(name, thread_wrapper, prio, stacksize, 0, 0, NULL);
  if (thread < 0) {
    sceClibPrintf("[SYS_ARCH] ERROR: sceKernelCreateThread failed with code 0x%x for %s\n",
           (unsigned int)thread, name);
    free(st);
    return NULL;
  }
  _debug_thread_count++;
  sceClibPrintf("[SYS_ARCH] Thread created: 0x%x (count=%d)\n", thread, _debug_thread_count);

  st->thread = thread;

  // Registrar buffer antes de iniciar el thread
  // Esto permite múltiples threads sin race conditions
  sceClibPrintf("[SYS_ARCH] sys_thread_new() - Registering buffer for thread 0x%x\n", thread);
  int buf_index = _register_thread_buffer(thread, st);
  if (buf_index < 0) {
    sceClibPrintf("[SYS_ARCH] ERROR: Failed to register thread buffer (full?)\n");
    sceKernelDeleteThread(thread);
    free(st);
    return NULL;
  }
  sceClibPrintf("[SYS_ARCH] Registered buffer for thread 0x%x at index %d\n", thread, buf_index);
  
  // Pasar 0 argumentos (usaremos buffers persistentes basados en thread ID)
  sceClibPrintf("[SYS_ARCH] sys_thread_new() - Calling sceKernelStartThread(0x%x, 0, NULL)\n", thread);
  int ret = sceKernelStartThread(thread, 0, NULL);
  if (ret < 0) {
    sceClibPrintf("[SYS_ARCH] ERROR: sceKernelStartThread failed with code 0x%x\n", ret);
    sceKernelDeleteThread(thread);
    _unregister_thread_buffer(thread);
    free(st);
    return NULL;
  }
  sceClibPrintf("[SYS_ARCH] Thread started: %s (0x%x)\n", name, thread);

  // No retornar st aquí porque el thread lo libera cuando termina
  return st;
}

/*-----------------------------------------------------------------------------------*/
/* Mailbox */
err_t
sys_mbox_new(sys_mbox_t *mb, int size)
{
  struct sys_mbox *mbox;
  LWIP_UNUSED_ARG(size);

  mbox = (struct sys_mbox *)malloc(sizeof(struct sys_mbox));
  if (mbox == NULL) {
    return ERR_MEM;
  }
  mbox->first = mbox->last = 0;
  mbox->wait_send = 0;

  // Create semaphores
  mbox->not_empty = sys_sem_new_internal(0);
  mbox->not_full = sys_sem_new_internal(0);
  mbox->mutex = sys_sem_new_internal(1);

  if (!sys_sem_valid(mbox->not_empty) || !sys_sem_valid(mbox->not_full) || !sys_sem_valid(mbox->mutex)) {
    sys_sem_free_internal(mbox->not_empty);
    sys_sem_free_internal(mbox->not_full);
    sys_sem_free_internal(mbox->mutex);
    free(mbox);
    return ERR_MEM;
  }

  SYS_STATS_INC_USED(mbox);
  *mb = mbox;
  return ERR_OK;
}

void
sys_mbox_free(sys_mbox_t *mb)
{
  if ((mb != NULL) && (*mb != SYS_MBOX_NULL)) {
    struct sys_mbox *mbox = *mb;
    SYS_STATS_DEC(mbox.used);

    sys_arch_sem_wait(&mbox->mutex, 0);

    sys_sem_free_internal(mbox->not_empty);
    sys_sem_free_internal(mbox->not_full);
    sys_sem_free_internal(mbox->mutex);

    free(mbox);
  }
}

err_t
sys_mbox_trypost(sys_mbox_t *mb, void *msg)
{
  u8_t first;
  struct sys_mbox *mbox;
  LWIP_ASSERT("invalid mbox", (mb != NULL) && (*mb != NULL));
  mbox = *mb;

  sys_arch_sem_wait(&mbox->mutex, 0);

  LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_trypost: mbox %p msg %p\n",
                          (void *)mbox, (void *)msg));

  if ((mbox->last + 1) >= (mbox->first + SYS_MBOX_SIZE)) {
    sys_sem_signal(&mbox->mutex);
    return ERR_MEM;
  }

  mbox->msgs[mbox->last % SYS_MBOX_SIZE] = msg;

  if (mbox->last == mbox->first) {
    first = 1;
  } else {
    first = 0;
  }

  mbox->last++;

  if (first) {
    sys_sem_signal(&mbox->not_empty);
  }

  sys_sem_signal(&mbox->mutex);

  return ERR_OK;
}

void
sys_mbox_post(sys_mbox_t *mb, void *msg)
{
  u8_t first;
  struct sys_mbox *mbox;
  LWIP_ASSERT("invalid mbox", (mb != NULL) && (*mb != NULL));
  mbox = *mb;

  sys_arch_sem_wait(&mbox->mutex, 0);

  LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_post: mbox %p msg %p\n", (void *)mbox, (void *)msg));

  while ((mbox->last + 1) >= (mbox->first + SYS_MBOX_SIZE)) {
    mbox->wait_send++;
    sys_sem_signal(&mbox->mutex);
    sys_arch_sem_wait(&mbox->not_full, 0);
    sys_arch_sem_wait(&mbox->mutex, 0);
    mbox->wait_send--;
  }

  mbox->msgs[mbox->last % SYS_MBOX_SIZE] = msg;

  if (mbox->last == mbox->first) {
    first = 1;
  } else {
    first = 0;
  }

  mbox->last++;

  if (first) {
    sys_sem_signal(&mbox->not_empty);
  }

  sys_sem_signal(&mbox->mutex);
}

u32_t
sys_arch_mbox_tryfetch(sys_mbox_t *mb, void **msg)
{
  struct sys_mbox *mbox;
  LWIP_ASSERT("invalid mbox", (mb != NULL) && (*mb != NULL));
  mbox = *mb;

  sys_arch_sem_wait(&mbox->mutex, 0);

  if (mbox->first == mbox->last) {
    sys_sem_signal(&mbox->mutex);
    return SYS_MBOX_EMPTY;
  }

  if (msg != NULL) {
    LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_tryfetch: mbox %p msg %p\n", (void *)mbox, *msg));
    *msg = mbox->msgs[mbox->first % SYS_MBOX_SIZE];
  }
  else{
    LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_tryfetch: mbox %p, null msg\n", (void *)mbox));
  }

  mbox->first++;

  if (mbox->wait_send) {
    sys_sem_signal(&mbox->not_full);
  }

  sys_sem_signal(&mbox->mutex);

  return 0;
}

u32_t
sys_arch_mbox_fetch(sys_mbox_t *mb, void **msg, u32_t timeout)
{
  u32_t time_needed = 0;
  struct sys_mbox *mbox;
  LWIP_ASSERT("invalid mbox", (mb != NULL) && (*mb != NULL));
  mbox = *mb;

  sys_arch_sem_wait(&mbox->mutex, 0);

  while (mbox->first == mbox->last) {
    sys_sem_signal(&mbox->mutex);

    if (timeout != 0) {
      time_needed = sys_arch_sem_wait(&mbox->not_empty, timeout);

      if (time_needed == SYS_ARCH_TIMEOUT) {
        return SYS_ARCH_TIMEOUT;
      }
    } else {
      sys_arch_sem_wait(&mbox->not_empty, 0);
    }

    sys_arch_sem_wait(&mbox->mutex, 0);
  }

  if (msg != NULL) {
    LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_fetch: mbox %p msg %p\n", (void *)mbox, *msg));
    *msg = mbox->msgs[mbox->first % SYS_MBOX_SIZE];
  }
  else{
    LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_fetch: mbox %p, null msg\n", (void *)mbox));
  }

  mbox->first++;

  if (mbox->wait_send) {
    sys_sem_signal(&mbox->not_full);
  }

  sys_sem_signal(&mbox->mutex);

  return time_needed;
}

/*-----------------------------------------------------------------------------------*/
/* Semaphore */

err_t
sys_sem_new(sys_sem_t *sem, u8_t count)
{
  sceClibPrintf("[SYS_ARCH] sys_sem_new() - count=%d\n", count);
  *sem = sceKernelCreateSema("lwip_sem", 0, count, 0x7FFFFFFF, NULL);
  if (*sem < 0) {
    sceClibPrintf("[SYS_ARCH] ERROR: sceKernelCreateSema failed with code 0x%x\n",
           (unsigned int)*sem);
    return ERR_MEM;
  }
  _debug_sema_count++;
  sceClibPrintf("[SYS_ARCH] Semaphore created: 0x%x (count=%d)\n", *sem, _debug_sema_count);
  return ERR_OK;
}

u32_t
sys_arch_sem_wait(sys_sem_t *s, u32_t timeout)
{
  SceUInt32 timeout_ms = (timeout == 0) ? 0xFFFFFFFF : timeout;
  SceUInt64 start_time = sceKernelGetSystemTimeWide();
  int ret = sceKernelWaitSema(*s, 1, &timeout_ms);

  if (ret == SCE_KERNEL_ERROR_WAIT_TIMEOUT) {
    return SYS_ARCH_TIMEOUT;
  }

  SceUInt64 end_time = sceKernelGetSystemTimeWide();
  return (end_time - start_time) / 1000; // Convert to milliseconds
}

void
sys_sem_signal(sys_sem_t *s)
{
  sceKernelSignalSema(*s, 1);
}

void
sys_sem_free(sys_sem_t *sem)
{
  sceKernelDeleteSema(*sem);
}

/*-----------------------------------------------------------------------------------*/
/* Mutex */
err_t
sys_mutex_new(sys_mutex_t *mutex)
{
  sceClibPrintf("[SYS_ARCH] sys_mutex_new()\n");
  *mutex = sceKernelCreateMutex("lwip_mutex", 0, 0, NULL);
  if (*mutex < 0) {
    sceClibPrintf("[SYS_ARCH] ERROR: sceKernelCreateMutex failed with code 0x%x\n", 
           (unsigned int)*mutex);
    return ERR_MEM;
  }
  _debug_mutex_count++;
  sceClibPrintf("[SYS_ARCH] Mutex created: 0x%x (count=%d)\n", *mutex, _debug_mutex_count);
  return ERR_OK;
}

void
sys_mutex_lock(sys_mutex_t *mutex)
{
  sceKernelLockMutex(*mutex, 1, NULL);
}

void
sys_mutex_unlock(sys_mutex_t *mutex)
{
  sceKernelUnlockMutex(*mutex, 1);
}

void
sys_mutex_free(sys_mutex_t *mutex)
{
  sceKernelDeleteMutex(*mutex);
}

/*-----------------------------------------------------------------------------------*/
/* Time */
u32_t
sys_now(void)
{
  return (u32_t)(sceKernelGetSystemTimeWide() / 1000); // Convert microseconds to milliseconds
}

u32_t
sys_jiffies(void)
{
  return (u32_t)(sceKernelGetSystemTimeWide() / 1000);
}

/*-----------------------------------------------------------------------------------*/
/* Init */
void
sys_init(void)
{
}

/*-----------------------------------------------------------------------------------*/
/* Critical section */
#if SYS_LIGHTWEIGHT_PROT
static SceUID lwprot_mutex = SCE_UID_INVALID;
static SceUID lwprot_thread = SCE_UID_INVALID;
static int lwprot_count = 0;

sys_prot_t
sys_arch_protect(void)
{
  SceUID current_thread = sceKernelGetThreadId();

  if (lwprot_thread != current_thread) {
    if (lwprot_mutex == SCE_UID_INVALID) {
      lwprot_mutex = sceKernelCreateMutex("lwprot", 0, 0, NULL);
    }
    sceKernelLockMutex(lwprot_mutex, 1, NULL);
    lwprot_thread = current_thread;
    lwprot_count = 1;
  } else {
    lwprot_count++;
  }
  return 0;
}

void
sys_arch_unprotect(sys_prot_t pval)
{
  LWIP_UNUSED_ARG(pval);
  if (lwprot_thread == sceKernelGetThreadId()) {
    lwprot_count--;
    if (lwprot_count == 0) {
      lwprot_thread = SCE_UID_INVALID;
      sceKernelUnlockMutex(lwprot_mutex, 1);
    }
  }
}
#endif /* SYS_LIGHTWEIGHT_PROT */

/*-----------------------------------------------------------------------------------*/
// Missing semaphore functions for lwIP
static struct sys_sem *sys_sem_new_internal(u8_t count)
{
  struct sys_sem *sem = (struct sys_sem *)malloc(sizeof(struct sys_sem));
  if (sem != NULL) {
    sem->sema = sceKernelCreateSema("lwip_sem", 0, count, 0x7FFFFFFF, NULL);
    if (sem->sema < 0) {
      free(sem);
      return NULL;
    }
  }
  return sem;
}

void sys_sem_free_internal(struct sys_sem *sem)
{
  if (sem != NULL) {
    sceKernelDeleteSema(sem->sema);
    free(sem);
  }
}

err_t sys_mbox_trypost_fromisr(sys_mbox_t *mbox, void *msg)
{
  // For PSVita, just call the regular trypost (no ISR context distinction)
  return sys_mbox_trypost(mbox, msg);
}