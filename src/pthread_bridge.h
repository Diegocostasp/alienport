#ifndef PTHREAD_BRIDGE_H
#define PTHREAD_BRIDGE_H

#include <pthread.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Bionic pthread ABI shims for LP64 (ARM64) */

int b_mutexattr_init(void *a);
int b_mutexattr_destroy(void *a);
int b_mutexattr_settype(void *a, int type);

int b_mutex_init(void *m, const void *attr);
int b_mutex_lock(void *m);
int b_mutex_unlock(void *m);
int b_mutex_trylock(void *m);
int b_mutex_destroy(void *m);

int b_condattr_init(void *a);
int b_condattr_destroy(void *a);
int b_condattr_setclock(void *a, clockid_t clock_id);

int b_cond_init(void *c, const void *attr);
int b_cond_destroy(void *c);
int b_cond_signal(void *c);
int b_cond_broadcast(void *c);
int b_cond_wait(void *c, void *m);
int b_cond_timedwait(void *c, void *m, const struct timespec *ts);

int b_rwlock_init(void *rw, const void *attr);
int b_rwlock_destroy(void *rw);
int b_rwlock_rdlock(void *rw);
int b_rwlock_wrlock(void *rw);
int b_rwlock_unlock(void *rw);

int b_pthread_once(pthread_once_t *once_control, void (*init_routine)(void));

#ifdef __cplusplus
}
#endif

#endif /* PTHREAD_BRIDGE_H */
