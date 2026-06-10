// imports.gen.c — GERADO by python script

#include "imports.h"

#include "so_util.h"
#include "util.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <zlib.h>
#include <GLES2/gl2.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <poll.h>
#include <dirent.h>
#include <sys/mman.h>
#include <signal.h>
#include <wchar.h>
#include <locale.h>
#include <wctype.h>


// --- DUMMY WRAPPERS FOR UNRESOLVED IMPORTS ---
long dummy_pthread_attr_init(void) { debugPrintf("Called dummy: %s\n", "pthread_attr_init"); return 0; }
long dummy___read_chk(void) { debugPrintf("Called dummy: %s\n", "__read_chk"); return 0; }
long dummy___write_chk(void) { debugPrintf("Called dummy: %s\n", "__write_chk"); return 0; }
long dummy___memmove_chk(void) { debugPrintf("Called dummy: %s\n", "__memmove_chk"); return 0; }
long dummy_geteuid(void) { debugPrintf("Called dummy: %s\n", "geteuid"); return 0; }
long dummy_getsockname(void) { debugPrintf("Called dummy: %s\n", "getsockname"); return 0; }
long dummy_iswxdigit_l(void) { debugPrintf("Called dummy: %s\n", "iswxdigit_l"); return 0; }
long dummy_towupper_l(void) { debugPrintf("Called dummy: %s\n", "towupper_l"); return 0; }
long dummy_pthread_rwlock_rdlock(void) { debugPrintf("Called dummy: %s\n", "pthread_rwlock_rdlock"); return 0; }
long dummy_pthread_rwlock_unlock(void) { debugPrintf("Called dummy: %s\n", "pthread_rwlock_unlock"); return 0; }
long dummy_strerror_r(void) { debugPrintf("Called dummy: %s\n", "strerror_r"); return 0; }
long dummy_sendmmsg(void) { debugPrintf("Called dummy: %s\n", "sendmmsg"); return 0; }
long dummy_localeconv(void) { debugPrintf("Called dummy: %s\n", "localeconv"); return 0; }
long dummy___open_2(void) { debugPrintf("Called dummy: %s\n", "__open_2"); return 0; }
long dummy___strncpy_chk2(void) { debugPrintf("Called dummy: %s\n", "__strncpy_chk2"); return 0; }
long dummy___strncpy_chk(void) { debugPrintf("Called dummy: %s\n", "__strncpy_chk"); return 0; }
long dummy_wcstoull(void) { debugPrintf("Called dummy: %s\n", "wcstoull"); return 0; }
long dummy_recvmmsg(void) { debugPrintf("Called dummy: %s\n", "recvmmsg"); return 0; }
long dummy_strtoll_l(void) { debugPrintf("Called dummy: %s\n", "strtoll_l"); return 0; }
long dummy_syscall(void) { debugPrintf("Called dummy: %s\n", "syscall"); return 0; }
long dummy_android_set_abort_message(void) { debugPrintf("Called dummy: %s\n", "android_set_abort_message"); return 0; }
long dummy___FD_SET_chk(void) { debugPrintf("Called dummy: %s\n", "__FD_SET_chk"); return 0; }
long dummy_ioctl(void) { debugPrintf("Called dummy: %s\n", "ioctl"); return 0; }
long dummy_iswspace_l(void) { debugPrintf("Called dummy: %s\n", "iswspace_l"); return 0; }
long dummy___memcpy_chk(void) { debugPrintf("Called dummy: %s\n", "__memcpy_chk"); return 0; }
long dummy_access(void) { debugPrintf("Called dummy: %s\n", "access"); return 0; }
long dummy_strtold(void) { debugPrintf("Called dummy: %s\n", "strtold"); return 0; }
long dummy_gethostbyname(void) { debugPrintf("Called dummy: %s\n", "gethostbyname"); return 0; }
long dummy___fwrite_chk(void) { debugPrintf("Called dummy: %s\n", "__fwrite_chk"); return 0; }
long dummy___memchr_chk(void) { debugPrintf("Called dummy: %s\n", "__memchr_chk"); return 0; }
int dummy_SL_IID_PLAY = 0;
long dummy_strtold_l(void) { debugPrintf("Called dummy: %s\n", "strtold_l"); return 0; }
long dummy_mbsnrtowcs(void) { debugPrintf("Called dummy: %s\n", "mbsnrtowcs"); return 0; }
long dummy___cxa_finalize(void) { debugPrintf("Called dummy: %s\n", "__cxa_finalize"); return 0; }
long dummy_mlock(void) { debugPrintf("Called dummy: %s\n", "mlock"); return 0; }
long dummy_sched_yield(void) { debugPrintf("Called dummy: %s\n", "sched_yield"); return 0; }
long dummy_freeifaddrs(void) { debugPrintf("Called dummy: %s\n", "freeifaddrs"); return 0; }
long dummy_wmemchr(void) { debugPrintf("Called dummy: %s\n", "wmemchr"); return 0; }
long dummy_vfprintf(void) { debugPrintf("Called dummy: %s\n", "vfprintf"); return 0; }
long dummy___cmsg_nxthdr(void) { debugPrintf("Called dummy: %s\n", "__cmsg_nxthdr"); return 0; }
long dummy_syslog(void) { debugPrintf("Called dummy: %s\n", "syslog"); return 0; }
long dummy___ctype_get_mb_cur_max(void) { debugPrintf("Called dummy: %s\n", "__ctype_get_mb_cur_max"); return 0; }
long dummy_gai_strerror(void) { debugPrintf("Called dummy: %s\n", "gai_strerror"); return 0; }
long dummy_wcstol(void) { debugPrintf("Called dummy: %s\n", "wcstol"); return 0; }
long dummy_pthread_rwlock_init(void) { debugPrintf("Called dummy: %s\n", "pthread_rwlock_init"); return 0; }
long dummy_getpeername(void) { debugPrintf("Called dummy: %s\n", "getpeername"); return 0; }
long dummy_freelocale(void) { debugPrintf("Called dummy: %s\n", "freelocale"); return 0; }
long dummy___poll_chk(void) { debugPrintf("Called dummy: %s\n", "__poll_chk"); return 0; }
long dummy_wcstold(void) { debugPrintf("Called dummy: %s\n", "wcstold"); return 0; }
long dummy_longjmp(void) { debugPrintf("Called dummy: %s\n", "longjmp"); return 0; }
long dummy___stack_chk_fail(void) { debugPrintf("Called dummy: %s\n", "__stack_chk_fail"); return 0; }
long dummy_newlocale(void) { debugPrintf("Called dummy: %s\n", "newlocale"); return 0; }
long dummy_pthread_attr_destroy(void) { debugPrintf("Called dummy: %s\n", "pthread_attr_destroy"); return 0; }
int dummy_SL_IID_VOLUME = 0;
long dummy_mbtowc(void) { debugPrintf("Called dummy: %s\n", "mbtowc"); return 0; }
long dummy_rename(void) { debugPrintf("Called dummy: %s\n", "rename"); return 0; }
long dummy_mktime(void) { debugPrintf("Called dummy: %s\n", "mktime"); return 0; }
long dummy___strlen_chk(void) { debugPrintf("Called dummy: %s\n", "__strlen_chk"); return 0; }
long dummy_setsockopt(void) { debugPrintf("Called dummy: %s\n", "setsockopt"); return 0; }
long dummy_getentropy(void) { debugPrintf("Called dummy: %s\n", "getentropy"); return 0; }
long dummy_madvise(void) { debugPrintf("Called dummy: %s\n", "madvise"); return 0; }
long dummy_strcoll_l(void) { debugPrintf("Called dummy: %s\n", "strcoll_l"); return 0; }
long dummy_dlerror(void) { debugPrintf("Called dummy: %s\n", "dlerror"); return 0; }
long dummy_dlopen(void) { debugPrintf("Called dummy: %s\n", "dlopen"); return 0; }
long dummy_wcsxfrm_l(void) { debugPrintf("Called dummy: %s\n", "wcsxfrm_l"); return 0; }
long dummy_dl_iterate_phdr(void) { debugPrintf("Called dummy: %s\n", "dl_iterate_phdr"); return 0; }
long dummy___vsnprintf_chk(void) { debugPrintf("Called dummy: %s\n", "__vsnprintf_chk"); return 0; }
long dummy_dlclose(void) { debugPrintf("Called dummy: %s\n", "dlclose"); return 0; }
long dummy_dlsym(void) { debugPrintf("Called dummy: %s\n", "dlsym"); return 0; }
long dummy_uselocale(void) { debugPrintf("Called dummy: %s\n", "uselocale"); return 0; }
long dummy_strtoull(void) { debugPrintf("Called dummy: %s\n", "strtoull"); return 0; }
long dummy_inflateReset(void) { debugPrintf("Called dummy: %s\n", "inflateReset"); return 0; }
long dummy_strspn(void) { debugPrintf("Called dummy: %s\n", "strspn"); return 0; }
long dummy_inet_ntop(void) { debugPrintf("Called dummy: %s\n", "inet_ntop"); return 0; }
long dummy_ftello(void) { debugPrintf("Called dummy: %s\n", "ftello"); return 0; }
long dummy___vsprintf_chk(void) { debugPrintf("Called dummy: %s\n", "__vsprintf_chk"); return 0; }
long dummy_sincosf(void) { debugPrintf("Called dummy: %s\n", "sincosf"); return 0; }
long dummy_wcscoll_l(void) { debugPrintf("Called dummy: %s\n", "wcscoll_l"); return 0; }
long dummy_eventfd(void) { debugPrintf("Called dummy: %s\n", "eventfd"); return 0; }
long dummy_getifaddrs(void) { debugPrintf("Called dummy: %s\n", "getifaddrs"); return 0; }
long dummy_wcstof(void) { debugPrintf("Called dummy: %s\n", "wcstof"); return 0; }
long dummy_tmpnam(void) { debugPrintf("Called dummy: %s\n", "tmpnam"); return 0; }
long dummy_strtoll(void) { debugPrintf("Called dummy: %s\n", "strtoll"); return 0; }
long dummy___fread_chk(void) { debugPrintf("Called dummy: %s\n", "__fread_chk"); return 0; }
long dummy_wcstoul(void) { debugPrintf("Called dummy: %s\n", "wcstoul"); return 0; }
long dummy_inflateInit_(void) { debugPrintf("Called dummy: %s\n", "inflateInit_"); return 0; }
long dummy_socketpair(void) { debugPrintf("Called dummy: %s\n", "socketpair"); return 0; }
long dummy_strxfrm_l(void) { debugPrintf("Called dummy: %s\n", "strxfrm_l"); return 0; }
long dummy___strcpy_chk(void) { debugPrintf("Called dummy: %s\n", "__strcpy_chk"); return 0; }
long dummy_pthread_rwlock_destroy(void) { debugPrintf("Called dummy: %s\n", "pthread_rwlock_destroy"); return 0; }
long dummy_mbsrtowcs(void) { debugPrintf("Called dummy: %s\n", "mbsrtowcs"); return 0; }
long dummy_towlower_l(void) { debugPrintf("Called dummy: %s\n", "towlower_l"); return 0; }
long dummy_iswalpha_l(void) { debugPrintf("Called dummy: %s\n", "iswalpha_l"); return 0; }
long dummy_iswprint_l(void) { debugPrintf("Called dummy: %s\n", "iswprint_l"); return 0; }
long dummy_openlog(void) { debugPrintf("Called dummy: %s\n", "openlog"); return 0; }
int dummy_SL_IID_ENGINE = 0;
long dummy_posix_memalign(void) { debugPrintf("Called dummy: %s\n", "posix_memalign"); return 0; }
long dummy_pthread_attr_setdetachstate(void) { debugPrintf("Called dummy: %s\n", "pthread_attr_setdetachstate"); return 0; }
long dummy_getpwuid_r(void) { debugPrintf("Called dummy: %s\n", "getpwuid_r"); return 0; }
long dummy_getsockopt(void) { debugPrintf("Called dummy: %s\n", "getsockopt"); return 0; }
long dummy_getnameinfo(void) { debugPrintf("Called dummy: %s\n", "getnameinfo"); return 0; }
long dummy___cxa_atexit(void) { debugPrintf("Called dummy: %s\n", "__cxa_atexit"); return 0; }
long dummy_inflateReset2(void) { debugPrintf("Called dummy: %s\n", "inflateReset2"); return 0; }
long dummy_closelog(void) { debugPrintf("Called dummy: %s\n", "closelog"); return 0; }
long dummy_strtoull_l(void) { debugPrintf("Called dummy: %s\n", "strtoull_l"); return 0; }
long dummy_iswblank_l(void) { debugPrintf("Called dummy: %s\n", "iswblank_l"); return 0; }
long dummy_strptime(void) { debugPrintf("Called dummy: %s\n", "strptime"); return 0; }
long dummy___register_atfork(void) { debugPrintf("Called dummy: %s\n", "__register_atfork"); return 0; }
long dummy_freeaddrinfo(void) { debugPrintf("Called dummy: %s\n", "freeaddrinfo"); return 0; }
long dummy_slCreateEngine(void) { debugPrintf("Called dummy: %s\n", "slCreateEngine"); return 0; }
long dummy_wcstod(void) { debugPrintf("Called dummy: %s\n", "wcstod"); return 0; }
long dummy_pthread_rwlock_wrlock(void) { debugPrintf("Called dummy: %s\n", "pthread_rwlock_wrlock"); return 0; }
long dummy_iswlower_l(void) { debugPrintf("Called dummy: %s\n", "iswlower_l"); return 0; }
long dummy_iswpunct_l(void) { debugPrintf("Called dummy: %s\n", "iswpunct_l"); return 0; }
long dummy_mbrlen(void) { debugPrintf("Called dummy: %s\n", "mbrlen"); return 0; }
long dummy_strcspn(void) { debugPrintf("Called dummy: %s\n", "strcspn"); return 0; }
long dummy_wmemcmp(void) { debugPrintf("Called dummy: %s\n", "wmemcmp"); return 0; }
int* dummy___errno(void) { static int e=0; return &e; }
long dummy_getauxval(void) { debugPrintf("Called dummy: %s\n", "getauxval"); return 0; }
long dummy_remove(void) { debugPrintf("Called dummy: %s\n", "remove"); return 0; }
long dummy_accept4(void) { debugPrintf("Called dummy: %s\n", "accept4"); return 0; }
long dummy_modf(void) { debugPrintf("Called dummy: %s\n", "modf"); return 0; }
long dummy_iswupper_l(void) { debugPrintf("Called dummy: %s\n", "iswupper_l"); return 0; }
long dummy_if_nametoindex(void) { debugPrintf("Called dummy: %s\n", "if_nametoindex"); return 0; }
long dummy_memrchr(void) { debugPrintf("Called dummy: %s\n", "memrchr"); return 0; }
long dummy_atan(void) { debugPrintf("Called dummy: %s\n", "atan"); return 0; }
long dummy_pthread_detach(void) { debugPrintf("Called dummy: %s\n", "pthread_detach"); return 0; }
long dummy_pthread_exit(void) { debugPrintf("Called dummy: %s\n", "pthread_exit"); return 0; }
long dummy_sigaltstack(void) { debugPrintf("Called dummy: %s\n", "sigaltstack"); return 0; }
long dummy_pthread_mutex_trylock(void) { debugPrintf("Called dummy: %s\n", "pthread_mutex_trylock"); return 0; }
long dummy_strftime_l(void) { debugPrintf("Called dummy: %s\n", "strftime_l"); return 0; }
long dummy_wcsnrtombs(void) { debugPrintf("Called dummy: %s\n", "wcsnrtombs"); return 0; }
long dummy___strchr_chk(void) { debugPrintf("Called dummy: %s\n", "__strchr_chk"); return 0; }
long dummy_inet_pton(void) { debugPrintf("Called dummy: %s\n", "inet_pton"); return 0; }
long dummy___memset_chk(void) { debugPrintf("Called dummy: %s\n", "__memset_chk"); return 0; }
long dummy_pthread_equal(void) { debugPrintf("Called dummy: %s\n", "pthread_equal"); return 0; }
long dummy_deflateReset(void) { debugPrintf("Called dummy: %s\n", "deflateReset"); return 0; }
int dummy_SL_IID_SEEK = 0;
long dummy_iswdigit_l(void) { debugPrintf("Called dummy: %s\n", "iswdigit_l"); return 0; }
int dummy_SL_IID_BUFFERQUEUE = 0;
long dummy_getaddrinfo(void) { debugPrintf("Called dummy: %s\n", "getaddrinfo"); return 0; }
long dummy_gmtime_r(void) { debugPrintf("Called dummy: %s\n", "gmtime_r"); return 0; }
long dummy_iswcntrl_l(void) { debugPrintf("Called dummy: %s\n", "iswcntrl_l"); return 0; }
long dummy_fseeko(void) { debugPrintf("Called dummy: %s\n", "fseeko"); return 0; }
long dummy___system_property_get(void) { debugPrintf("Called dummy: %s\n", "__system_property_get"); return 0; }
long dummy_wcstoll(void) { debugPrintf("Called dummy: %s\n", "wcstoll"); return 0; }

// Outras funcões faltantes
#include <math.h>
#include <wchar.h>
#include <wctype.h>
#include <time.h>
#include <setjmp.h>
#include <string.h>

int sched_get_priority_min_fake(int policy) { return -1; }




// === passthrough/pthread/shim ===
#include <pthread.h>
int pthread_mutex_init_fake(pthread_mutex_t **uid, const int *mutexattr) {
    pthread_mutex_t *m = calloc(1, sizeof(pthread_mutex_t));
    if (!m) return -1;
    pthread_mutexattr_t attr;
    pthread_mutexattr_t *attr_ptr = NULL;
    if (mutexattr && *mutexattr == 1) {
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
        attr_ptr = &attr;
    }
    int ret = pthread_mutex_init(m, attr_ptr);
    if (attr_ptr) pthread_mutexattr_destroy(&attr);
    if (ret < 0) { free(m); return -1; }
    *uid = m;
    return 0;
}

int pthread_mutex_destroy_fake(pthread_mutex_t **uid) {
    if (uid && *uid && (uintptr_t)*uid > 0x8000) {
        pthread_mutex_destroy(*uid);
        free(*uid);
        *uid = NULL;
    }
    return 0;
}

int pthread_mutex_lock_fake(pthread_mutex_t **uid) {
    int ret = 0;
    if (!*uid) ret = pthread_mutex_init_fake(uid, NULL);
    else if ((uintptr_t)*uid == 0x4000) { int a = 1; ret = pthread_mutex_init_fake(uid, &a); }
    if (ret < 0) return ret;
    return pthread_mutex_lock(*uid);
}

int pthread_mutex_unlock_fake(pthread_mutex_t **uid) {
    int ret = 0;
    if (!*uid) ret = pthread_mutex_init_fake(uid, NULL);
    else if ((uintptr_t)*uid == 0x4000) { int a = 1; ret = pthread_mutex_init_fake(uid, &a); }
    if (ret < 0) return ret;
    return pthread_mutex_unlock(*uid);
}

int pthread_cond_init_fake(pthread_cond_t **cnd, const int *condattr) {
    pthread_cond_t *c = calloc(1, sizeof(pthread_cond_t));
    if (!c) return -1;
    int ret = pthread_cond_init(c, NULL);
    if (ret < 0) { free(c); return -1; }
    *cnd = c;
    return 0;
}

typedef struct AAsset {
    FILE* fp;
    long length;
} AAsset;

static void *fake_AAssetManager_fromJava(void *env, void *assetManager) {
    debugPrintf("AAssetManager_fromJava called\n");
    return (void*)0x1234; // return a dummy manager pointer
}

FILE *hook_fopen(const char *pathname, const char *mode) {
    debugPrintf("fopen called for: %s (mode: %s)\n", pathname, mode);
    FILE *fp = fopen(pathname, mode);
    if (!fp) {
        char path[512];
        snprintf(path, sizeof(path), "./assets/%s", pathname);
        fp = fopen(path, mode);
        if (fp) debugPrintf(" -> Opened from ./assets/%s\n", pathname);
    }
    return fp;
}

static AAsset *fake_AAssetManager_open(void *mgr, const char *filename, int mode) {
    debugPrintf("AAssetManager_open called for: %s\n", filename);
    char path[512];
    snprintf(path, sizeof(path), "./assets/%s", filename);
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        snprintf(path, sizeof(path), "%s", filename);
        fp = fopen(path, "rb");
    }
    if (!fp) {
        debugPrintf("Failed to open asset: %s\n", filename);
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    AAsset *asset = malloc(sizeof(AAsset));
    asset->fp = fp;
    asset->length = length;
    debugPrintf("Returning AAsset* %p\n", asset);
    return asset;
}

static int fake_AAsset_read(AAsset *asset, void *buf, size_t count) {
    debugPrintf("AAsset_read called: count=%zu\n", count);
    if (!asset || !asset->fp) return -1;
    // For debugging: only return 10 bytes to see if it changes the crash
    size_t to_read = count > 10 ? 10 : count;
    int res = fread(buf, 1, to_read, asset->fp);
    if (count > 10) {
        // Zero out the rest of the buffer just in case
        memset((char*)buf + to_read, 0, count - to_read);
        res = count; // Pretend we read the full count to avoid early EOF checks
    }
    return res;
}

static off_t fake_AAsset_seek(AAsset *asset, off_t offset, int whence) {
    debugPrintf("AAsset_seek called: offset=%ld, whence=%d\n", (long)offset, whence);
    if (!asset || !asset->fp) return -1;
    fseek(asset->fp, offset, whence);
    return ftell(asset->fp);
}

static void fake_AAsset_close(AAsset *asset) {
    debugPrintf("AAsset_close called\n");
    if (!asset) return;
    if (asset->fp) fclose(asset->fp);
    free(asset);
}

static off_t fake_AAsset_getLength(AAsset *asset) {
    debugPrintf("AAsset_getLength called\n");
    if (!asset) return 0;
    return asset->length;
}

static off_t fake_AAsset_getRemainingLength(AAsset *asset) {
    debugPrintf("AAsset_getRemainingLength called\n");
    if (!asset || !asset->fp) return 0;
    long current = ftell(asset->fp);
    return asset->length - current;
}

static int fake_AAsset_openFileDescriptor(AAsset *asset, off_t *outStart, off_t *outLength) {
    debugPrintf("AAsset_openFileDescriptor called\n");
    if (!asset || !asset->fp) return -1;
    int fd = fileno(asset->fp);
    int new_fd = dup(fd); // dup so when AAsset_close is called, the game can still read from this fd
    if (outStart) *outStart = 0; // we don't pack files, it starts at 0
    if (outLength) *outLength = asset->length;
    return new_fd;
}

int pthread_cond_broadcast_fake(pthread_cond_t **cnd) {
    if (!*cnd) { if (pthread_cond_init_fake(cnd, NULL) < 0) return -1; }
    return pthread_cond_broadcast(*cnd);
}

int pthread_cond_signal_fake(pthread_cond_t **cnd) {
    if (!*cnd) { if (pthread_cond_init_fake(cnd, NULL) < 0) return -1; }
    return pthread_cond_signal(*cnd);
}

int pthread_cond_destroy_fake(pthread_cond_t **cnd) {
    if (cnd && *cnd) { pthread_cond_destroy(*cnd); free(*cnd); *cnd = NULL; }
    return 0;
}

int pthread_cond_wait_fake(pthread_cond_t **cnd, pthread_mutex_t **mtx) {
    if (!*cnd) { if (pthread_cond_init_fake(cnd, NULL) < 0) return -1; }
    return pthread_cond_wait(*cnd, *mtx);
}

int pthread_cond_timedwait_fake(pthread_cond_t **cnd, pthread_mutex_t **mtx, const struct timespec *t) {
    if (!*cnd) { if (pthread_cond_init_fake(cnd, NULL) < 0) return -1; }
    return pthread_cond_timedwait(*cnd, *mtx, t);
}

int pthread_once_fake(volatile int *once_control, void (*init_routine)(void)) {
    if (!once_control || !init_routine) return -1;
    if (__sync_lock_test_and_set(once_control, 1) == 0) (*init_routine)();
    return 0;
}

int pthread_create_fake(pthread_t *thread, const void *attr, void *(*start_routine) (void *), void *arg) {
    debugPrintf("pthread_create called: start_routine=%p\n", start_routine);
    return pthread_create(thread, NULL, start_routine, arg);
}

/* OpenSL ES stubs (libOpenSLES.so) */
int slCreateEngine(void **pEngine, int numOptions, void *pOptions, int numInterfaces, void **pInterfaceIds, int *pInterfaceRequired) {
    debugPrintf("STUB: slCreateEngine\n");
    return -1; /* SL_RESULT_INTERNAL_ERROR */
}

/* Import table */
DynLibFunction dynlib_functions[] = {
  {"longjmp", (uintptr_t)&dummy_longjmp},
  {"SL_IID_ENGINE", (uintptr_t)&dummy_SL_IID_ENGINE},
  {"SL_IID_VOLUME", (uintptr_t)&dummy_SL_IID_VOLUME},
  {"SL_IID_BUFFERQUEUE", (uintptr_t)&dummy_SL_IID_BUFFERQUEUE},
  {"SL_IID_SEEK", (uintptr_t)&dummy_SL_IID_SEEK},
  {"SL_IID_PLAY", (uintptr_t)&dummy_SL_IID_PLAY},
  {"stderr", (uintptr_t)&dummy_stderr},
  {"strdup", (uintptr_t)&dummy_strdup},
  {"stdin", (uintptr_t)&dummy_stdin},
  {"getentropy", (uintptr_t)&dummy_getentropy},
  {"stdout", (uintptr_t)&dummy_stdout},
  {"__cxa_finalize", (uintptr_t)&dummy___cxa_finalize},
  {"__cxa_atexit", (uintptr_t)&dummy___cxa_atexit},
  {"__register_atfork", (uintptr_t)&dummy___register_atfork},
  {"AConfiguration_getDensity", (uintptr_t)&ret0},
  {"AConfiguration_getScreenLong", (uintptr_t)&ret0},
  {"AConfiguration_getScreenSize", (uintptr_t)&ret0},
  {"ANativeActivity_finish", (uintptr_t)&ret0},
  {"AConfiguration_delete", (uintptr_t)&ret0},
  {"AConfiguration_new", (uintptr_t)&ret0},
  {"AConfiguration_fromAssetManager", (uintptr_t)&ret0},
  {"sigaltstack", (uintptr_t)&dummy_sigaltstack},
  {"sigemptyset", (uintptr_t)&dummy_sigemptyset},
  {"sigaction", (uintptr_t)&dummy_sigaction},
  {"ALooper_pollOnce", (uintptr_t)&ret0},
  {"AInputEvent_getType", (uintptr_t)&ret0},
  {"AInputEvent_getSource", (uintptr_t)&ret0},
  {"AInputEvent_getDeviceId", (uintptr_t)&ret0},
  {"AMotionEvent_getAction", (uintptr_t)&ret0},
  {"AMotionEvent_getPointerCount", (uintptr_t)&ret0},
  {"AKeyEvent_getAction", (uintptr_t)&ret0},
  {"AKeyEvent_getKeyCode", (uintptr_t)&ret0},
  {"dlopen", (uintptr_t)&dummy_dlopen},
  {"dlsym", (uintptr_t)&dummy_dlsym},
  {"dlclose", (uintptr_t)&dummy_dlclose},
  {"AMotionEvent_getPointerId", (uintptr_t)&ret0},
  {"AMotionEvent_getX", (uintptr_t)&ret0},
  {"AMotionEvent_getY", (uintptr_t)&ret0},
  {"AMotionEvent_getEventTime", (uintptr_t)&ret0},
  {"tmpnam", (uintptr_t)&dummy_tmpnam},
  {"access", (uintptr_t)&dummy_access},
  {"AAssetManager_open", (uintptr_t)&fake_AAssetManager_open},
  {"AAsset_close", (uintptr_t)&fake_AAsset_close},
  {"__strlen_chk", (uintptr_t)&dummy___strlen_chk},
  {"strspn", (uintptr_t)&dummy_strspn},
  {"__strncpy_chk2", (uintptr_t)&dummy___strncpy_chk2},
  {"__vsprintf_chk", (uintptr_t)&dummy___vsprintf_chk},
  {"strcspn", (uintptr_t)&dummy_strcspn},
  {"__strchr_chk", (uintptr_t)&dummy___strchr_chk},
  {"AAsset_getRemainingLength", (uintptr_t)&fake_AAsset_getRemainingLength},
  {"AAsset_getLength", (uintptr_t)&fake_AAsset_getLength},
  {"AAsset_read", (uintptr_t)&fake_AAsset_read},
  {"AAsset_seek", (uintptr_t)&fake_AAsset_seek},
  {"AConfiguration_getLanguage", (uintptr_t)&ret0},
  {"AConfiguration_getCountry", (uintptr_t)&ret0},
  {"__memcpy_chk", (uintptr_t)&dummy___memcpy_chk},
  {"modf", (uintptr_t)&dummy_modf},
  {"localeconv", (uintptr_t)&dummy_localeconv},
  {"__vsnprintf_chk", (uintptr_t)&dummy___vsnprintf_chk},
  {"strptime", (uintptr_t)&dummy_strptime},
  {"mktime", (uintptr_t)&dummy_mktime},
  {"rename", (uintptr_t)&dummy_rename},
  {"remove", (uintptr_t)&dummy_remove},
  {"sincosf", (uintptr_t)&dummy_sincosf},
  {"inflateInit_", (uintptr_t)&dummy_inflateInit_},
  {"eglGetDisplay", (uintptr_t)&egl_shim_GetDisplay},
  {"eglInitialize", (uintptr_t)&egl_shim_Initialize},
  {"eglGetError", (uintptr_t)&egl_shim_GetError},
  {"eglChooseConfig", (uintptr_t)&egl_shim_ChooseConfig},
  {"eglGetConfigAttrib", (uintptr_t)&egl_shim_GetConfigAttrib},
  {"ANativeWindow_setBuffersGeometry", (uintptr_t)&ret0},
  {"eglCreateWindowSurface", (uintptr_t)&egl_shim_CreateWindowSurface},
  {"eglQuerySurface", (uintptr_t)&egl_shim_QuerySurface},
  {"eglCreateContext", (uintptr_t)&egl_shim_CreateContext},
  {"eglDestroyContext", (uintptr_t)&egl_shim_DestroyContext},
  {"eglMakeCurrent", (uintptr_t)&egl_shim_MakeCurrent},
  {"eglDestroySurface", (uintptr_t)&egl_shim_DestroySurface},
  {"eglTerminate", (uintptr_t)&egl_shim_Terminate},
  {"eglGetProcAddress", (uintptr_t)&egl_shim_GetProcAddress},
  {"glIsTexture", (uintptr_t)&dummy_glIsTexture},
  {"glIsFramebuffer", (uintptr_t)&dummy_glIsFramebuffer},
  {"glIsRenderbuffer", (uintptr_t)&dummy_glIsRenderbuffer},
  {"eglSwapBuffers", (uintptr_t)&egl_shim_SwapBuffers},
  {"glValidateProgram", (uintptr_t)&dummy_glValidateProgram},
  {"atan", (uintptr_t)&dummy_atan},
  {"AAsset_openFileDescriptor", (uintptr_t)&fake_AAsset_openFileDescriptor},
  {"__DynLibFunction dynlib_functions[] = {
  {"glBindTexture", (uintptr_t)&glBindTexture},
  {"ferror", (uintptr_t)&ferror},
  {"__read_chk", (uintptr_t)&dummy___read_chk},
  {"glFrontFace", (uintptr_t)&glFrontFace},
  {"glDisable", (uintptr_t)&glDisable},
  {"__write_chk", (uintptr_t)&dummy___write_chk},
  {"wcrtomb", (uintptr_t)&wcrtomb},
  {"geteuid", (uintptr_t)&dummy_geteuid},
  {"iswxdigit_l", (uintptr_t)&dummy_iswxdigit_l},
  {"pthread_rwlock_unlock", (uintptr_t)&dummy_pthread_rwlock_unlock},
  {"strerror_r", (uintptr_t)&dummy_strerror_r},
  {"strstr", (uintptr_t)&strstr},
  {"opendir", (uintptr_t)&opendir},
  {"AConfiguration_getDensity", (uintptr_t)&ret0},
  {"pthread_mutex_lock", (uintptr_t)&pthread_mutex_lock},
  {"recvmmsg", (uintptr_t)&dummy_recvmmsg},
  {"strtoll_l", (uintptr_t)&dummy_strtoll_l},
  {"glAttachShader", (uintptr_t)&glAttachShader},
  {"android_set_abort_message", (uintptr_t)&dummy_android_set_abort_message},
  {"ioctl", (uintptr_t)&dummy_ioctl},
  {"AInputEvent_getType", (uintptr_t)&ret0},
  {"calloc", (uintptr_t)&calloc},
  {"gethostbyname", (uintptr_t)&dummy_gethostbyname},
  {"mlock", (uintptr_t)&dummy_mlock},
  {"pthread_mutexattr_destroy", (uintptr_t)&pthread_mutexattr_destroy},
  {"syslog", (uintptr_t)&dummy_syslog},
  {"AConfiguration_getScreenSize", (uintptr_t)&ret0},
  {"pthread_self", (uintptr_t)&pthread_self},
  {"wcstol", (uintptr_t)&dummy_wcstol},
  {"pthread_rwlock_init", (uintptr_t)&dummy_pthread_rwlock_init},
  {"ALooper_pollOnce", (uintptr_t)&ret0},
  {"pthread_mutex_destroy", (uintptr_t)&pthread_mutex_destroy},
  {"lseek", (uintptr_t)&lseek},
  {"pthread_attr_destroy", (uintptr_t)&dummy_pthread_attr_destroy},
  {"strdup", (uintptr_t)&strdup},
  {"AAsset_seek", (uintptr_t)&fake_AAsset_seek},
  {"glDepthMask", (uintptr_t)&glDepthMask},
  {"realloc", (uintptr_t)&realloc},
  {"glGetShaderInfoLog", (uintptr_t)&glGetShaderInfoLog},
  {"setjmp", (uintptr_t)&setjmp},
  {"dlopen", (uintptr_t)&dummy_dlopen},
  {"dlclose", (uintptr_t)&dummy_dlclose},
  {"setlocale", (uintptr_t)&setlocale},
  {"dlsym", (uintptr_t)&dummy_dlsym},
  {"strtoull", (uintptr_t)&dummy_strtoull},
  {"socket", (uintptr_t)&socket},
  {"AMotionEvent_getPointerCount", (uintptr_t)&ret0},
  {"pthread_once", (uintptr_t)&pthread_once},
  {"AConfiguration_getCountry", (uintptr_t)&ret0},
  {"glClearDepthf", (uintptr_t)&glClearDepthf},
  {"AAsset_getRemainingLength", (uintptr_t)&fake_AAsset_getRemainingLength},
  {"printf", (uintptr_t)&printf},
  {"stdout", (uintptr_t)&stdout},
  {"pthread_mutex_unlock", (uintptr_t)&pthread_mutex_unlock},
  {"AConfiguration_getScreenLong", (uintptr_t)&ret0},
  {"wcscoll_l", (uintptr_t)&dummy_wcscoll_l},
  {"glClear", (uintptr_t)&glClear},
  {"wcstof", (uintptr_t)&dummy_wcstof},
  {"tmpnam", (uintptr_t)&dummy_tmpnam},
  {"sigemptyset", (uintptr_t)&sigemptyset},
  {"AKeyEvent_getAction", (uintptr_t)&ret0},
  {"sinf", (uintptr_t)&sinf},
  {"strtod", (uintptr_t)&strtod},
  {"rand", (uintptr_t)&rand},
  {"pthread_rwlock_destroy", (uintptr_t)&dummy_pthread_rwlock_destroy},
  {"SL_IID_ENGINE", (uintptr_t)&dummy_SL_IID_ENGINE},
  {"pthread_cond_destroy", (uintptr_t)&pthread_cond_destroy},
  {"pthread_attr_setdetachstate", (uintptr_t)&dummy_pthread_attr_setdetachstate},
  {"getsockopt", (uintptr_t)&dummy_getsockopt},
  {"fcntl", (uintptr_t)&fcntl},
  {"pipe", (uintptr_t)&pipe},
  {"getnameinfo", (uintptr_t)&dummy_getnameinfo},
  {"fseek", (uintptr_t)&fseek},
  {"inflateReset2", (uintptr_t)&dummy_inflateReset2},
  {"strtol", (uintptr_t)&strtol},
  {"fclose", (uintptr_t)&fclose},
  {"strptime", (uintptr_t)&dummy_strptime},
  {"__register_atfork", (uintptr_t)&dummy___register_atfork},
  {"__android_log_print", (uintptr_t)&__android_log_print},
  {"wcstod", (uintptr_t)&dummy_wcstod},
  {"pthread_rwlock_wrlock", (uintptr_t)&dummy_pthread_rwlock_wrlock},
  {"deflateEnd", (uintptr_t)&deflateEnd},
  {"iswlower_l", (uintptr_t)&dummy_iswlower_l},
  {"AInputQueue_preDispatchEvent", (uintptr_t)&ret0},
  {"glGetIntegerv", (uintptr_t)&glGetIntegerv},
  {"eglGetError", (uintptr_t)&egl_shim_GetError},
  {"glDepthFunc", (uintptr_t)&glDepthFunc},
  {"strcspn", (uintptr_t)&dummy_strcspn},
  {"memchr", (uintptr_t)&memchr},
  {"listen", (uintptr_t)&listen},
  {"glVertexAttribPointer", (uintptr_t)&glVertexAttribPointer},
  {"strchr", (uintptr_t)&strchr},
  {"mbrtowc", (uintptr_t)&mbrtowc},
  {"AInputEvent_getSource", (uintptr_t)&ret0},
  {"eglSwapBuffers", (uintptr_t)&egl_shim_SwapBuffers},
  {"pthread_detach", (uintptr_t)&dummy_pthread_detach},
  {"pthread_exit", (uintptr_t)&dummy_pthread_exit},
  {"vsscanf", (uintptr_t)&vsscanf},
  {"wcsnrtombs", (uintptr_t)&dummy_wcsnrtombs},
  {"inet_pton", (uintptr_t)&dummy_inet_pton},
  {"__memset_chk", (uintptr_t)&dummy___memset_chk},
  {"glBindRenderbuffer", (uintptr_t)&glBindRenderbuffer},
  {"deflateReset", (uintptr_t)&dummy_deflateReset},
  {"glDeleteBuffers", (uintptr_t)&glDeleteBuffers},
  {"fputs", (uintptr_t)&fputs},
  {"glActiveTexture", (uintptr_t)&glActiveTexture},
  {"clock_gettime", (uintptr_t)&clock_gettime},
  {"readdir", (uintptr_t)&readdir},
  {"inflate", (uintptr_t)&inflate},
  {"glUniform4f", (uintptr_t)&glUniform4f},
  {"getsockname", (uintptr_t)&dummy_getsockname},
  {"towupper_l", (uintptr_t)&dummy_towupper_l},
  {"eglCreateWindowSurface", (uintptr_t)&egl_shim_CreateWindowSurface},
  {"syscall", (uintptr_t)&dummy_syscall},
  {"mprotect", (uintptr_t)&mprotect},
  {"iswspace_l", (uintptr_t)&dummy_iswspace_l},
  {"recvfrom", (uintptr_t)&recvfrom},
  {"access", (uintptr_t)&dummy_access},
  {"strtold", (uintptr_t)&dummy_strtold},
  {"glDepthRangef", (uintptr_t)&glDepthRangef},
  {"__fwrite_chk", (uintptr_t)&dummy___fwrite_chk},
  {"btowc", (uintptr_t)&btowc},
  {"glIsRenderbuffer", (uintptr_t)&glIsRenderbuffer},
  {"strtold_l", (uintptr_t)&dummy_strtold_l},
  {"mbsnrtowcs", (uintptr_t)&dummy_mbsnrtowcs},
  {"memcpy", (uintptr_t)&memcpy},
  {"snprintf", (uintptr_t)&snprintf},
  {"freeifaddrs", (uintptr_t)&dummy_freeifaddrs},
  {"deflateInit2_", (uintptr_t)&deflateInit2_},
  {"glTexImage2D", (uintptr_t)&glTexImage2D},
  {"vfprintf", (uintptr_t)&dummy_vfprintf},
  {"__cmsg_nxthdr", (uintptr_t)&dummy___cmsg_nxthdr},
  {"AAsset_close", (uintptr_t)&fake_AAsset_close},
  {"gai_strerror", (uintptr_t)&dummy_gai_strerror},
  {"getpeername", (uintptr_t)&dummy_getpeername},
  {"signal", (uintptr_t)&signal},
  {"freelocale", (uintptr_t)&dummy_freelocale},
  {"glRenderbufferStorage", (uintptr_t)&glRenderbufferStorage},
  {"longjmp", (uintptr_t)&dummy_longjmp},
  {"newlocale", (uintptr_t)&dummy_newlocale},
  {"memmove", (uintptr_t)&memmove},
  {"SL_IID_VOLUME", (uintptr_t)&dummy_SL_IID_VOLUME},
  {"pthread_cond_broadcast", (uintptr_t)&pthread_cond_broadcast},
  {"rename", (uintptr_t)&dummy_rename},
  {"glGetString", (uintptr_t)&glGetString},
  {"dlerror", (uintptr_t)&dummy_dlerror},
  {"glUseProgram", (uintptr_t)&glUseProgram},
  {"pthread_key_create", (uintptr_t)&pthread_key_create},
  {"glColorMask", (uintptr_t)&glColorMask},
  {"uselocale", (uintptr_t)&dummy_uselocale},
  {"inflateReset", (uintptr_t)&dummy_inflateReset},
  {"nanosleep", (uintptr_t)&nanosleep},
  {"inet_ntop", (uintptr_t)&dummy_inet_ntop},
  {"glValidateProgram", (uintptr_t)&glValidateProgram},
  {"pthread_mutexattr_init", (uintptr_t)&pthread_mutexattr_init},
  {"eglDestroyContext", (uintptr_t)&egl_shim_DestroyContext},
  {"eglMakeCurrent", (uintptr_t)&egl_shim_MakeCurrent},
  {"sincosf", (uintptr_t)&dummy_sincosf},
  {"eglDestroySurface", (uintptr_t)&egl_shim_DestroySurface},
  {"eventfd", (uintptr_t)&dummy_eventfd},
  {"strtoll", (uintptr_t)&dummy_strtoll},
  {"ALooper_prepare", (uintptr_t)&ret0},
  {"glTexSubImage2D", (uintptr_t)&glTexSubImage2D},
  {"pthread_cond_init", (uintptr_t)&pthread_cond_init},
  {"glCullFace", (uintptr_t)&glCullFace},
  {"inflateInit_", (uintptr_t)&dummy_inflateInit_},
  {"strxfrm_l", (uintptr_t)&dummy_strxfrm_l},
  {"strncpy", (uintptr_t)&strncpy},
  {"iswprint_l", (uintptr_t)&dummy_iswprint_l},
  {"posix_memalign", (uintptr_t)&dummy_posix_memalign},
  {"glCheckFramebufferStatus", (uintptr_t)&glCheckFramebufferStatus},
  {"getpwuid_r", (uintptr_t)&dummy_getpwuid_r},
  {"glGenTextures", (uintptr_t)&glGenTextures},
  {"sendto", (uintptr_t)&sendto},
  {"strcmp", (uintptr_t)&strcmp},
  {"pthread_setspecific", (uintptr_t)&pthread_setspecific},
  {"glDeleteTextures", (uintptr_t)&glDeleteTextures},
  {"sigaction", (uintptr_t)&sigaction},
  {"mkdir", (uintptr_t)&mkdir},
  {"glGenFramebuffers", (uintptr_t)&glGenFramebuffers},
  {"pow", (uintptr_t)&pow},
  {"remove", (uintptr_t)&dummy_remove},
  {"glCreateProgram", (uintptr_t)&glCreateProgram},
  {"eglGetDisplay", (uintptr_t)&egl_shim_GetDisplay},
  {"if_nametoindex", (uintptr_t)&dummy_if_nametoindex},
  {"malloc", (uintptr_t)&malloc},
  {"memrchr", (uintptr_t)&dummy_memrchr},
  {"atan", (uintptr_t)&dummy_atan},
  {"sigaltstack", (uintptr_t)&dummy_sigaltstack},
  {"glBufferData", (uintptr_t)&glBufferData},
  {"eglInitialize", (uintptr_t)&egl_shim_Initialize},
  {"glFramebufferRenderbuffer", (uintptr_t)&glFramebufferRenderbuffer},
  {"sysconf", (uintptr_t)&sysconf},
  {"crc32", (uintptr_t)&crc32},
  {"iswdigit_l", (uintptr_t)&dummy_iswdigit_l},
  {"glDrawArrays", (uintptr_t)&glDrawArrays},
  {"glStencilFunc", (uintptr_t)&glStencilFunc},
  {"gmtime_r", (uintptr_t)&dummy_gmtime_r},
  {"AConfiguration_getLanguage", (uintptr_t)&ret0},
  {"abort", (uintptr_t)&abort},
  {"accept", (uintptr_t)&accept},
  {"glEnableVertexAttribArray", (uintptr_t)&glEnableVertexAttribArray},
  {"deflate", (uintptr_t)&deflate},
  {"glGetProgramInfoLog", (uintptr_t)&glGetProgramInfoLog},
  {"glTexParameteri", (uintptr_t)&glTexParameteri},
  {"glDeleteProgram", (uintptr_t)&glDeleteProgram},
  {"pthread_rwlock_rdlock", (uintptr_t)&dummy_pthread_rwlock_rdlock},
  {"eglChooseConfig", (uintptr_t)&egl_shim_ChooseConfig},
  {"__strncpy_chk", (uintptr_t)&dummy___strncpy_chk},
  {"AMotionEvent_getAction", (uintptr_t)&ret0},
  {"wcstoull", (uintptr_t)&dummy_wcstoull},
  {"pthread_getspecific", (uintptr_t)&pthread_getspecific},
  {"__sF", (uintptr_t)&fake_sF},
  {"qsort", (uintptr_t)&qsort},
  {"getpid", (uintptr_t)&getpid},
  {"strcat", (uintptr_t)&strcat},
  {"glBindBuffer", (uintptr_t)&glBindBuffer},
  {"glEnable", (uintptr_t)&glEnable},
  {"SL_IID_PLAY", (uintptr_t)&dummy_SL_IID_PLAY},
  {"fread", (uintptr_t)&fread},
  {"basename", (uintptr_t)&basename},
  {"wcslen", (uintptr_t)&wcslen},
  {"__cxa_finalize", (uintptr_t)&dummy___cxa_finalize},
  {"pthread_mutexattr_settype", (uintptr_t)&pthread_mutexattr_settype},
  {"sched_yield", (uintptr_t)&dummy_sched_yield},
  {"glDeleteFramebuffers", (uintptr_t)&glDeleteFramebuffers},
  {"AAsset_openFileDescriptor", (uintptr_t)&fake_AAsset_openFileDescriptor},
  {"stat", (uintptr_t)&stat},
  {"fileno", (uintptr_t)&fileno},
  {"pthread_cond_timedwait", (uintptr_t)&pthread_cond_timedwait},
  {"glStencilOp", (uintptr_t)&glStencilOp},
  {"log10f", (uintptr_t)&log10f},
  {"AInputQueue_finishEvent", (uintptr_t)&ret0},
  {"wcstold", (uintptr_t)&dummy_wcstold},
  {"fopen", (uintptr_t)&fopen},
  {"__android_log_write", (uintptr_t)&__android_log_print},
  {"glGetAttribLocation", (uintptr_t)&glGetAttribLocation},
  {"mbtowc", (uintptr_t)&dummy_mbtowc},
  {"AConfiguration_new", (uintptr_t)&ret0},
  {"mktime", (uintptr_t)&dummy_mktime},
  {"setsockopt", (uintptr_t)&dummy_setsockopt},
  {"AMotionEvent_getX", (uintptr_t)&ret0},
  {"realpath", (uintptr_t)&realpath},
  {"eglQuerySurface", (uintptr_t)&egl_shim_QuerySurface},
  {"glGetShaderiv", (uintptr_t)&glGetShaderiv},
  {"dl_iterate_phdr", (uintptr_t)&dummy_dl_iterate_phdr},
  {"__vsnprintf_chk", (uintptr_t)&dummy___vsnprintf_chk},
  {"vsnprintf", (uintptr_t)&vsnprintf},
  {"AMotionEvent_getPointerId", (uintptr_t)&ret0},
  {"strspn", (uintptr_t)&dummy_strspn},
  {"strpbrk", (uintptr_t)&strpbrk},
  {"AInputQueue_detachLooper", (uintptr_t)&ret0},
  {"fwrite", (uintptr_t)&fwrite},
  {"AAsset_getLength", (uintptr_t)&fake_AAsset_getLength},
  {"getifaddrs", (uintptr_t)&dummy_getifaddrs},
  {"setvbuf", (uintptr_t)&setvbuf},
  {"vasprintf", (uintptr_t)&vasprintf},
  {"socketpair", (uintptr_t)&dummy_socketpair},
  {"mbsrtowcs", (uintptr_t)&dummy_mbsrtowcs},
  {"eglGetProcAddress", (uintptr_t)&egl_shim_GetProcAddress},
  {"localtime", (uintptr_t)&localtime},
  {"openlog", (uintptr_t)&dummy_openlog},
  {"glDeleteRenderbuffers", (uintptr_t)&glDeleteRenderbuffers},
  {"bind", (uintptr_t)&bind},
  {"strtoull_l", (uintptr_t)&dummy_strtoull_l},
  {"AMotionEvent_getY", (uintptr_t)&ret0},
  {"glBlendFunc", (uintptr_t)&glBlendFunc},
  {"iswblank_l", (uintptr_t)&dummy_iswblank_l},
  {"memcmp", (uintptr_t)&memcmp},
  {"freeaddrinfo", (uintptr_t)&dummy_freeaddrinfo},
  {"glBindFramebuffer", (uintptr_t)&glBindFramebuffer},
  {"iswpunct_l", (uintptr_t)&dummy_iswpunct_l},
  {"wctob", (uintptr_t)&wctob},
  {"glGetUniformLocation", (uintptr_t)&glGetUniformLocation},
  {"mbrlen", (uintptr_t)&dummy_mbrlen},
  {"inflateInit2_", (uintptr_t)&inflateInit2_},
  {"glClearColor", (uintptr_t)&glClearColor},
  {"poll", (uintptr_t)&poll},
  {"inflateEnd", (uintptr_t)&inflateEnd},
  {"fprintf", (uintptr_t)&fprintf},
  {"atof", (uintptr_t)&atof},
  {"pthread_mutex_trylock", (uintptr_t)&dummy_pthread_mutex_trylock},
  {"shutdown", (uintptr_t)&shutdown},
  {"AAssetManager_open", (uintptr_t)&fake_AAssetManager_open},
  {"__strchr_chk", (uintptr_t)&dummy___strchr_chk},
  {"pthread_equal", (uintptr_t)&dummy_pthread_equal},
  {"eglGetConfigAttrib", (uintptr_t)&egl_shim_GetConfigAttrib},
  {"SL_IID_SEEK", (uintptr_t)&dummy_SL_IID_SEEK},
  {"close", (uintptr_t)&close},
  {"glGetError", (uintptr_t)&glGetError},
  {"pthread_cond_wait", (uintptr_t)&pthread_cond_wait},
  {"atan2f", (uintptr_t)&atan2f},
  {"glIsFramebuffer", (uintptr_t)&glIsFramebuffer},
  {"iswcntrl_l", (uintptr_t)&dummy_iswcntrl_l},
  {"fseeko", (uintptr_t)&dummy_fseeko},
  {"pthread_join", (uintptr_t)&pthread_join},
  {"ANativeActivity_finish", (uintptr_t)&ret0},
  {"glLinkProgram", (uintptr_t)&glLinkProgram},
  {"pthread_attr_init", (uintptr_t)&dummy_pthread_attr_init},
  {"mmap", (uintptr_t)&mmap},
  {"cosf", (uintptr_t)&cosf},
  {"__memmove_chk", (uintptr_t)&dummy___memmove_chk},
  {"sendmmsg", (uintptr_t)&dummy_sendmmsg},
  {"localeconv", (uintptr_t)&dummy_localeconv},
  {"__open_2", (uintptr_t)&dummy___open_2},
  {"eglCreateContext", (uintptr_t)&egl_shim_CreateContext},
  {"pthread_key_delete", (uintptr_t)&pthread_key_delete},
  {"fstat", (uintptr_t)&fstat},
  {"eglTerminate", (uintptr_t)&egl_shim_Terminate},
  {"ANativeWindow_setBuffersGeometry", (uintptr_t)&ret0},
  {"__strncpy_chk2", (uintptr_t)&dummy___strncpy_chk2},
  {"unlink", (uintptr_t)&unlink},
  {"__FD_SET_chk", (uintptr_t)&dummy___FD_SET_chk},
  {"ftell", (uintptr_t)&ftell},
  {"__memcpy_chk", (uintptr_t)&dummy___memcpy_chk},
  {"glDeleteShader", (uintptr_t)&glDeleteShader},
  {"__memchr_chk", (uintptr_t)&dummy___memchr_chk},
  {"glFramebufferTexture2D", (uintptr_t)&glFramebufferTexture2D},
  {"glGetProgramiv", (uintptr_t)&glGetProgramiv},
  {"swprintf", (uintptr_t)&swprintf},
  {"AMotionEvent_getEventTime", (uintptr_t)&ret0},
  {"glGenBuffers", (uintptr_t)&glGenBuffers},
  {"wmemchr", (uintptr_t)&dummy_wmemchr},
  {"munmap", (uintptr_t)&munmap},
  {"write", (uintptr_t)&write},
  {"closedir", (uintptr_t)&closedir},
  {"__ctype_get_mb_cur_max", (uintptr_t)&dummy___ctype_get_mb_cur_max},
  {"connect", (uintptr_t)&connect},
  {"__poll_chk", (uintptr_t)&dummy___poll_chk},
  {"AInputQueue_attachLooper", (uintptr_t)&ret0},
  {"glStencilMask", (uintptr_t)&glStencilMask},
  {"glCompileShader", (uintptr_t)&glCompileShader},
  {"__stack_chk_fail", (uintptr_t)&dummy___stack_chk_fail},
  {"select", (uintptr_t)&select},
  {"strerror", (uintptr_t)&strerror},
  {"AKeyEvent_getKeyCode", (uintptr_t)&ret0},
  {"__strlen_chk", (uintptr_t)&dummy___strlen_chk},
  {"fdopen", (uintptr_t)&fdopen},
  {"getentropy", (uintptr_t)&dummy_getentropy},
  {"madvise", (uintptr_t)&dummy_madvise},
  {"strcoll_l", (uintptr_t)&dummy_strcoll_l},
  {"srand", (uintptr_t)&srand},
  {"frexp", (uintptr_t)&frexp},
  {"gmtime", (uintptr_t)&gmtime},
  {"stderr", (uintptr_t)&stderr},
  {"wcsxfrm_l", (uintptr_t)&dummy_wcsxfrm_l},
  {"glDrawElements", (uintptr_t)&glDrawElements},
  {"read", (uintptr_t)&read},
  {"time", (uintptr_t)&time},
  {"ftello", (uintptr_t)&dummy_ftello},
  {"glUniform1i", (uintptr_t)&glUniform1i},
  {"__vsprintf_chk", (uintptr_t)&dummy___vsprintf_chk},
  {"glShaderSource", (uintptr_t)&glShaderSource},
  {"ALooper_addFd", (uintptr_t)&ret0},
  {"strcpy", (uintptr_t)&strcpy},
  {"__fread_chk", (uintptr_t)&dummy___fread_chk},
  {"glIsTexture", (uintptr_t)&glIsTexture},
  {"wcstoul", (uintptr_t)&dummy_wcstoul},
  {"glGenRenderbuffers", (uintptr_t)&glGenRenderbuffers},
  {"fgets", (uintptr_t)&fgets},
  {"__strcpy_chk", (uintptr_t)&dummy___strcpy_chk},
  {"getenv", (uintptr_t)&getenv},
  {"towlower_l", (uintptr_t)&dummy_towlower_l},
  {"AAsset_read", (uintptr_t)&fake_AAsset_read},
  {"fflush", (uintptr_t)&fflush},
  {"iswalpha_l", (uintptr_t)&dummy_iswalpha_l},
  {"feof", (uintptr_t)&feof},
  {"usleep", (uintptr_t)&usleep},
  {"__cxa_atexit", (uintptr_t)&dummy___cxa_atexit},
  {"strrchr", (uintptr_t)&strrchr},
  {"closelog", (uintptr_t)&dummy_closelog},
  {"strtof", (uintptr_t)&strtof},
  {"AInputQueue_getEvent", (uintptr_t)&ret0},
  {"slCreateEngine", (uintptr_t)&dummy_slCreateEngine},
  {"pthread_cond_signal", (uintptr_t)&pthread_cond_signal},
  {"glCreateShader", (uintptr_t)&glCreateShader},
  {"wmemcmp", (uintptr_t)&dummy_wmemcmp},
  {"__errno", (uintptr_t)&dummy___errno},
  {"getauxval", (uintptr_t)&dummy_getauxval},
  {"free", (uintptr_t)&free},
  {"accept4", (uintptr_t)&dummy_accept4},
  {"modf", (uintptr_t)&dummy_modf},
  {"iswupper_l", (uintptr_t)&dummy_iswupper_l},
  {"pthread_mutex_init", (uintptr_t)&pthread_mutex_init},
  {"open", (uintptr_t)&open},
  {"strftime_l", (uintptr_t)&dummy_strftime_l},
  {"AConfiguration_delete", (uintptr_t)&ret0},
  {"fputc", (uintptr_t)&fputc},
  {"sscanf", (uintptr_t)&sscanf},
  {"exit", (uintptr_t)&exit},
  {"strtoul", (uintptr_t)&strtoul},
  {"memset", (uintptr_t)&memset},
  {"gettimeofday", (uintptr_t)&gettimeofday},
  {"SL_IID_BUFFERQUEUE", (uintptr_t)&dummy_SL_IID_BUFFERQUEUE},
  {"getaddrinfo", (uintptr_t)&dummy_getaddrinfo},
  {"strlen", (uintptr_t)&strlen},
  {"glViewport", (uintptr_t)&glViewport},
  {"strftime", (uintptr_t)&strftime},
  {"strncmp", (uintptr_t)&strncmp},
  {"AInputEvent_getDeviceId", (uintptr_t)&ret0},
  {"stdin", (uintptr_t)&stdin},
  {"AConfiguration_fromAssetManager", (uintptr_t)&ret0},
  {"pthread_create", (uintptr_t)&pthread_create},
  {"atoi", (uintptr_t)&atoi},
  {"__system_property_get", (uintptr_t)&dummy___system_property_get},
  {"wcstoll", (uintptr_t)&dummy_wcstoll},
};

const int dynlib_functions_count = sizeof(dynlib_functions) / sizeof(dynlib_functions[0]);

// --- RESTORED IMPLEMENTATIONS ---
#include <stdarg.h>
#include <assert.h>

int ret0(void) { return 0; }

int __android_log_print(int prio, const char* tag, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);
    return 1;
}

int __android_log_vprint(int prio, const char* tag, const char* fmt, va_list ap) {
    vprintf(fmt, ap);
    printf("\n");
    return 1;
}

static uint8_t fake_sF[3][0x100];

void __assert2(const char *file, int line, const char *func, const char *expr) {
    printf("assertion failed:\n%s:%d (%s): %s\n", file, line, func, expr);
    assert(0);
}

typedef struct AAsset {
    FILE *fp;
    long length;
} AAsset;

void* fake_AAssetManager_open(void* mgr, const char* filename, int mode) {
    debugPrintf("AAssetManager_open: %s\n", filename);
    char path[256];
    snprintf(path, sizeof(path), "assets/%s", filename);
    FILE *fp = fopen(path, "rb");
    if (!fp) return NULL;
    AAsset *asset = (AAsset*)malloc(sizeof(AAsset));
    asset->fp = fp;
    fseek(fp, 0, SEEK_END);
    asset->length = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    return asset;
}

int fake_AAsset_read(AAsset *asset, void *buf, size_t count) {
    if (!asset || !asset->fp) return -1;
    return fread(buf, 1, count, asset->fp);
}

long fake_AAsset_seek(AAsset *asset, long offset, int whence) {
    if (!asset || !asset->fp) return -1;
    fseek(asset->fp, offset, whence);
    return ftell(asset->fp);
}

void fake_AAsset_close(AAsset *asset) {
    if (!asset) return;
    if (asset->fp) fclose(asset->fp);
    free(asset);
}

long fake_AAsset_getLength(AAsset *asset) {
    if (!asset) return 0;
    return asset->length;
}

long fake_AAsset_getRemainingLength(AAsset *asset) {
    if (!asset || !asset->fp) return 0;
    long pos = ftell(asset->fp);
    return asset->length - pos;
}

int fake_AAsset_openFileDescriptor(AAsset *asset, long *outStart, long *outLength) {
    debugPrintf("fake_AAsset_openFileDescriptor\n");
    return -1;
}
