/*
 * pthread_preload.c - Preload shim para corrigir tamanho de mutex/cond
 *
 * Carregado via LD_PRELOAD antes do jogo. Intercepta funções de pthread
 * que o jogo (libsyberia1.so) chama esperando o layout biônico do Android,
 * onde mutex=4 bytes e cond=4 bytes. No Linux/glibc eles são 40/48 bytes.
 *
 * Estratégia: Armazenamos no "slot" do objeto do jogo um ponteiro (8 bytes)
 * para um objeto glibc real alocado no heap.
 *
 * Compilar: veja CMakeLists.txt
 */

#define _GNU_SOURCE
#include <pthread.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

/* Funções reais obtidas via RTLD_NEXT */
static int (*real_mutex_init)(pthread_mutex_t *, const pthread_mutexattr_t *) = NULL;
static int (*real_mutex_lock)(pthread_mutex_t *) = NULL;
static int (*real_mutex_unlock)(pthread_mutex_t *) = NULL;
static int (*real_mutex_trylock)(pthread_mutex_t *) = NULL;
static int (*real_mutex_destroy)(pthread_mutex_t *) = NULL;
static int (*real_cond_init)(pthread_cond_t *, const pthread_condattr_t *) = NULL;
static int (*real_cond_wait)(pthread_cond_t *, pthread_mutex_t *) = NULL;
static int (*real_cond_timedwait)(pthread_cond_t *, pthread_mutex_t *, const struct timespec *) = NULL;
static int (*real_cond_signal)(pthread_cond_t *) = NULL;
static int (*real_cond_broadcast)(pthread_cond_t *) = NULL;
static int (*real_cond_destroy)(pthread_cond_t *) = NULL;

static void __attribute__((constructor)) shim_init(void) {
    real_mutex_init     = dlsym(RTLD_NEXT, "pthread_mutex_init");
    real_mutex_lock     = dlsym(RTLD_NEXT, "pthread_mutex_lock");
    real_mutex_unlock   = dlsym(RTLD_NEXT, "pthread_mutex_unlock");
    real_mutex_trylock  = dlsym(RTLD_NEXT, "pthread_mutex_trylock");
    real_mutex_destroy  = dlsym(RTLD_NEXT, "pthread_mutex_destroy");
    real_cond_init      = dlsym(RTLD_NEXT, "pthread_cond_init");
    real_cond_wait      = dlsym(RTLD_NEXT, "pthread_cond_wait");
    real_cond_timedwait = dlsym(RTLD_NEXT, "pthread_cond_timedwait");
    real_cond_signal    = dlsym(RTLD_NEXT, "pthread_cond_signal");
    real_cond_broadcast = dlsym(RTLD_NEXT, "pthread_cond_broadcast");
    real_cond_destroy   = dlsym(RTLD_NEXT, "pthread_cond_destroy");
}

/* Magic para detectar se o slot foi inicializado */
#define SHIM_MAGIC 0xDEADBEEFCAFEBABEULL

/*
 * O jogo aloca pthread_mutex_t com o tamanho do Android (tipicamente 4 bytes
 * em 32-bit, 8 bytes em 64-bit). Na prática em ARM64 o biônico usa 40 bytes
 * também, mas a ordem dos campos é diferente, o que causa o SIGBUS.
 *
 * Usamos os primeiros 16 bytes do objeto para guardar:
 *   bytes 0-7:  magic (SHIM_MAGIC)
 *   bytes 8-15: ponteiro para o objeto glibc real (malloc'd)
 */
static void *get_real_mutex(void *obj) {
    if (!obj) return NULL;
    uint64_t *magic = (uint64_t *)obj;
    void **ptr = (void **)((char *)obj + 8);
    if (*magic != SHIM_MAGIC || !*ptr) {
        *ptr = calloc(1, sizeof(pthread_mutex_t));
        if (*ptr) real_mutex_init((pthread_mutex_t *)*ptr, NULL);
        *magic = SHIM_MAGIC;
    }
    return *ptr;
}

static void *get_real_cond(void *obj) {
    if (!obj) return NULL;
    uint64_t *magic = (uint64_t *)obj;
    void **ptr = (void **)((char *)obj + 8);
    if (*magic != SHIM_MAGIC || !*ptr) {
        *ptr = calloc(1, sizeof(pthread_cond_t));
        if (*ptr) real_cond_init((pthread_cond_t *)*ptr, NULL);
        *magic = SHIM_MAGIC;
    }
    return *ptr;
}

/* ---- pthread_mutex ---- */
int pthread_mutex_init(pthread_mutex_t *m, const pthread_mutexattr_t *a) {
    if (!m) return 0;
    uint64_t *magic = (uint64_t *)m;
    void **ptr = (void **)((char *)m + 8);
    if (*magic == SHIM_MAGIC && *ptr) {
        real_mutex_destroy((pthread_mutex_t *)*ptr);
    } else {
        *ptr = calloc(1, sizeof(pthread_mutex_t));
    }
    *magic = SHIM_MAGIC;
    return *ptr ? real_mutex_init((pthread_mutex_t *)*ptr, a) : ENOMEM;
}
int pthread_mutex_lock(pthread_mutex_t *m)    { return real_mutex_lock(get_real_mutex(m)); }
int pthread_mutex_unlock(pthread_mutex_t *m)  { return real_mutex_unlock(get_real_mutex(m)); }
int pthread_mutex_trylock(pthread_mutex_t *m) { return real_mutex_trylock(get_real_mutex(m)); }
int pthread_mutex_destroy(pthread_mutex_t *m) {
    if (!m) return 0;
    uint64_t *magic = (uint64_t *)m;
    void **ptr = (void **)((char *)m + 8);
    if (*magic == SHIM_MAGIC && *ptr) {
        int r = real_mutex_destroy((pthread_mutex_t *)*ptr);
        free(*ptr); *ptr = NULL; *magic = 0;
        return r;
    }
    return 0;
}

/* ---- pthread_cond ---- */
int pthread_cond_init(pthread_cond_t *c, const pthread_condattr_t *a) {
    if (!c) return 0;
    uint64_t *magic = (uint64_t *)c;
    void **ptr = (void **)((char *)c + 8);
    if (*magic == SHIM_MAGIC && *ptr) {
        real_cond_destroy((pthread_cond_t *)*ptr);
    } else {
        *ptr = calloc(1, sizeof(pthread_cond_t));
    }
    *magic = SHIM_MAGIC;
    return *ptr ? real_cond_init((pthread_cond_t *)*ptr, a) : ENOMEM;
}
int pthread_cond_wait(pthread_cond_t *c, pthread_mutex_t *m) {
    return real_cond_wait(get_real_cond(c), get_real_mutex(m));
}
int pthread_cond_timedwait(pthread_cond_t *c, pthread_mutex_t *m, const struct timespec *t) {
    return real_cond_timedwait(get_real_cond(c), get_real_mutex(m), t);
}
int pthread_cond_signal(pthread_cond_t *c)    { return real_cond_signal(get_real_cond(c)); }
int pthread_cond_broadcast(pthread_cond_t *c) { return real_cond_broadcast(get_real_cond(c)); }
int pthread_cond_destroy(pthread_cond_t *c) {
    if (!c) return 0;
    uint64_t *magic = (uint64_t *)c;
    void **ptr = (void **)((char *)c + 8);
    if (*magic == SHIM_MAGIC && *ptr) {
        int r = real_cond_destroy((pthread_cond_t *)*ptr);
        free(*ptr); *ptr = NULL; *magic = 0;
        return r;
    }
    return 0;
}
