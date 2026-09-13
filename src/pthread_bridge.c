/*
 * pthread_bridge.c — Ponte de ABI pthread Bionic -> glibc.
 *
 * Baseado no fix provado de Beach Buggy Racing / reVC / GTA SA.
 * No Bionic LP64 (ARM64), pthread_mutex_t tem 40 bytes enquanto no glibc tem 48 bytes.
 * Para evitar corrupção de memória ou deadlocks, armazenamos no início do storage Bionic
 * um ponteiro para um objeto glibc real alocado no heap.
 */

#include "pthread_bridge.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>

static pthread_mutex_t g_lock;

__attribute__((constructor)) static void init_glock(void) {
  pthread_mutexattr_t a;
  pthread_mutexattr_init(&a);
  pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(&g_lock, &a);
  pthread_mutexattr_destroy(&a);
}

/* ---------------- Mutex Attributes ---------------- */
int b_mutexattr_init(void *a) {
  if (a)
    *(int *)a = 0;
  return 0;
}

int b_mutexattr_destroy(void *a) {
  (void)a;
  return 0;
}

int b_mutexattr_settype(void *a, int type) {
  if (a)
    *(int *)a = type;
  return 0;
}

/* Rastreamento estrito de objetos alocados pela bridge para evitar free() em inteiros Bionic */
#define MAX_TRACKED_OBJS 8192
static void *g_tracked_objs[MAX_TRACKED_OBJS];
static int g_num_tracked = 0;

static void track_obj(void *ptr) {
  if (g_num_tracked < MAX_TRACKED_OBJS) {
    g_tracked_objs[g_num_tracked++] = ptr;
  }
}

static int is_tracked_obj(void *ptr) {
  if (!ptr || (uintptr_t)ptr < 0x10000) return 0;
  for (int i = 0; i < g_num_tracked; i++) {
    if (g_tracked_objs[i] == ptr) return 1;
  }
  return 0;
}

static void untrack_obj(void *ptr) {
  for (int i = 0; i < g_num_tracked; i++) {
    if (g_tracked_objs[i] == ptr) {
      g_tracked_objs[i] = g_tracked_objs[--g_num_tracked];
      return;
    }
  }
}

static pthread_mutex_t *new_recursive_mutex(void) {
  pthread_mutex_t *r = (pthread_mutex_t *)calloc(1, sizeof(pthread_mutex_t));
  pthread_mutexattr_t a;
  pthread_mutexattr_init(&a);
  pthread_mutexattr_settype(&a, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(r, &a);
  pthread_mutexattr_destroy(&a);
  track_obj(r);
  return r;
}

static pthread_mutex_t *mtx_real(void *m) {
  if (!m) return NULL;
  pthread_mutex_t **slot = (pthread_mutex_t **)m;
  if (is_tracked_obj(*slot))
    return *slot;
  pthread_mutex_lock(&g_lock);
  if (!is_tracked_obj(*slot))
    *slot = new_recursive_mutex();
  pthread_mutex_unlock(&g_lock);
  return *slot;
}

int b_mutex_init(void *m, const void *attr) {
  (void)attr;
  if (!m) return 0;
  pthread_mutex_t **slot = (pthread_mutex_t **)m;
  pthread_mutex_lock(&g_lock);
  if (is_tracked_obj(*slot)) {
    pthread_mutex_destroy(*slot);
    void *to_free = *slot;
    untrack_obj(to_free);
    free(to_free);
  }
  *slot = new_recursive_mutex();
  pthread_mutex_unlock(&g_lock);
  return 0;
}

int b_mutex_lock(void *m) {
  pthread_mutex_t *real = mtx_real(m);
  return real ? pthread_mutex_lock(real) : 0;
}

int b_mutex_unlock(void *m) {
  pthread_mutex_t *real = mtx_real(m);
  return real ? pthread_mutex_unlock(real) : 0;
}

int b_mutex_trylock(void *m) {
  pthread_mutex_t *real = mtx_real(m);
  return real ? pthread_mutex_trylock(real) : 0;
}

int b_mutex_destroy(void *m) {
  if (!m) return 0;
  pthread_mutex_t **slot = (pthread_mutex_t **)m;
  pthread_mutex_lock(&g_lock);
  if (is_tracked_obj(*slot)) {
    pthread_mutex_destroy(*slot);
    void *to_free = *slot;
    untrack_obj(to_free);
    *slot = NULL;
    free(to_free);
  } else {
    *slot = NULL;
  }
  pthread_mutex_unlock(&g_lock);
  return 0;
}

/* ---------------- Condition Variables ---------------- */

int b_condattr_init(void *a) {
  if (a) *(int *)a = 0;
  return 0;
}

int b_condattr_destroy(void *a) {
  (void)a;
  return 0;
}

int b_condattr_setclock(void *a, clockid_t clock_id) {
  if (a) *(int *)a = (int)clock_id;
  return 0;
}

static pthread_cond_t *new_monotonic_cond(void) {
  pthread_cond_t *r = (pthread_cond_t *)calloc(1, sizeof(pthread_cond_t));
  pthread_condattr_t a;
  pthread_condattr_init(&a);
  pthread_condattr_setclock(&a, CLOCK_MONOTONIC);
  pthread_cond_init(r, &a);
  pthread_condattr_destroy(&a);
  track_obj(r);
  return r;
}

static pthread_cond_t *cond_real(void *c) {
  if (!c) return NULL;
  pthread_cond_t **slot = (pthread_cond_t **)c;
  if (is_tracked_obj(*slot))
    return *slot;
  pthread_mutex_lock(&g_lock);
  if (!is_tracked_obj(*slot))
    *slot = new_monotonic_cond();
  pthread_mutex_unlock(&g_lock);
  return *slot;
}

int b_cond_init(void *c, const void *attr) {
  (void)attr;
  if (!c) return 0;
  pthread_cond_t **slot = (pthread_cond_t **)c;
  pthread_mutex_lock(&g_lock);
  if (is_tracked_obj(*slot)) {
    pthread_cond_destroy(*slot);
    void *to_free = *slot;
    untrack_obj(to_free);
    free(to_free);
  }
  *slot = new_monotonic_cond();
  pthread_mutex_unlock(&g_lock);
  return 0;
}

int b_cond_destroy(void *c) {
  if (!c) return 0;
  pthread_cond_t **slot = (pthread_cond_t **)c;
  pthread_mutex_lock(&g_lock);
  if (is_tracked_obj(*slot)) {
    pthread_cond_destroy(*slot);
    void *to_free = *slot;
    untrack_obj(to_free);
    *slot = NULL;
    free(to_free);
  } else {
    *slot = NULL;
  }
  pthread_mutex_unlock(&g_lock);
  return 0;
}

int b_cond_signal(void *c) {
  pthread_cond_t *real = cond_real(c);
  return real ? pthread_cond_signal(real) : 0;
}

int b_cond_broadcast(void *c) {
  pthread_cond_t *real = cond_real(c);
  return real ? pthread_cond_broadcast(real) : 0;
}

int b_cond_wait(void *c, void *m) {
  pthread_cond_t *cr = cond_real(c);
  pthread_mutex_t *mr = mtx_real(m);
  return (cr && mr) ? pthread_cond_wait(cr, mr) : 0;
}

int b_cond_timedwait(void *c, void *m, const struct timespec *ts) {
  pthread_cond_t *cr = cond_real(c);
  pthread_mutex_t *mr = mtx_real(m);
  return (cr && mr) ? pthread_cond_timedwait(cr, mr, ts) : 0;
}

/* ---------------- Read-Write Locks ---------------- */

static pthread_rwlock_t *rw_real(void *rw) {
  if (!rw) return NULL;
  pthread_rwlock_t **slot = (pthread_rwlock_t **)rw;
  if (is_tracked_obj(*slot))
    return *slot;
  pthread_mutex_lock(&g_lock);
  if (!is_tracked_obj(*slot)) {
    pthread_rwlock_t *rr = (pthread_rwlock_t *)calloc(1, sizeof(pthread_rwlock_t));
    pthread_rwlock_init(rr, NULL);
    track_obj(rr);
    *slot = rr;
  }
  pthread_mutex_unlock(&g_lock);
  return *slot;
}

int b_rwlock_init(void *rw, const void *attr) {
  (void)attr;
  if (!rw) return 0;
  pthread_rwlock_t **slot = (pthread_rwlock_t **)rw;
  pthread_rwlock_t *r = (pthread_rwlock_t *)calloc(1, sizeof(pthread_rwlock_t));
  pthread_rwlock_init(r, NULL);
  pthread_mutex_lock(&g_lock);
  if (is_tracked_obj(*slot)) {
    pthread_rwlock_destroy(*slot);
    void *to_free = *slot;
    untrack_obj(to_free);
    free(to_free);
  }
  track_obj(r);
  *slot = r;
  pthread_mutex_unlock(&g_lock);
  return 0;
}

int b_rwlock_destroy(void *rw) {
  if (!rw) return 0;
  pthread_rwlock_t **slot = (pthread_rwlock_t **)rw;
  pthread_mutex_lock(&g_lock);
  if (is_tracked_obj(*slot)) {
    pthread_rwlock_destroy(*slot);
    void *to_free = *slot;
    untrack_obj(to_free);
    *slot = NULL;
    free(to_free);
  } else {
    *slot = NULL;
  }
  pthread_mutex_unlock(&g_lock);
  return 0;
}

int b_rwlock_rdlock(void *rw) {
  pthread_rwlock_t *real = rw_real(rw);
  return real ? pthread_rwlock_rdlock(real) : 0;
}

int b_rwlock_wrlock(void *rw) {
  pthread_rwlock_t *real = rw_real(rw);
  return real ? pthread_rwlock_wrlock(real) : 0;
}

int b_rwlock_unlock(void *rw) {
  pthread_rwlock_t *real = rw_real(rw);
  return real ? pthread_rwlock_unlock(real) : 0;
}

int b_once(void *once_ctl, void (*init)(void)) {
  if (!once_ctl || !init) return 0;
  volatile int *st = (volatile int *)once_ctl;
  pthread_mutex_lock(&g_lock);
  while (*st == 1) {
    pthread_mutex_unlock(&g_lock);
    usleep(200);
    pthread_mutex_lock(&g_lock);
  }
  if (*st == 0) {
    *st = 1;
    pthread_mutex_unlock(&g_lock);
    init();
    pthread_mutex_lock(&g_lock);
    *st = 2;
  }
  pthread_mutex_unlock(&g_lock);
  return 0;
}

int b_pthread_once(pthread_once_t *once_control, void (*init_routine)(void)) {
  return b_once((void *)once_control, init_routine);
}

int my_sigaction(int sig, const void *act, void *old) {
  if (!act) return sigaction(sig, NULL, (struct sigaction *)old);
  void *h = *(void * const *)act; /* sa_handler / sa_sigaction @ offset 0 */
  struct sigaction g;
  memset(&g, 0, sizeof(g));
  g.sa_sigaction = (void (*)(int, siginfo_t *, void *))h;
  sigemptyset(&g.sa_mask);
  g.sa_flags = SA_SIGINFO | SA_RESTART;
  int rr = sigaction(sig, &g, (struct sigaction *)old);
  return rr < 0 ? 0 : rr;
}

