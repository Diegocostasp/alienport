/* imports_extra.c -- Additional symbol resolution for Alien Shooter */
#define _GNU_SOURCE
#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>
#include <dirent.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/auxv.h>
#include <sys/syscall.h>
#include <sys/eventfd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <net/if.h>
#include <poll.h>
#include <syslog.h>
#include <time.h>
#include <pwd.h>
#include <libgen.h>
#include <link.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <zlib.h>
#include <setjmp.h>
#include "so_util.h"
#include "util.h"

/* EGL globals from main.c */
extern EGLDisplay g_egl_display;
extern EGLSurface g_egl_surface;
extern EGLContext g_egl_context;
extern int g_screen_w, g_screen_h;
extern SDL_Window *g_sdl_window;
extern SDL_GLContext g_sdl_glctx;

/* Fake EGL handles - game thinks these are real */
static EGLDisplay fake_display = (EGLDisplay)0xE6D1;
static EGLSurface fake_surface = (EGLSurface)0xE6D2;
static EGLContext fake_context = (EGLContext)0xE6D3;

/* Hooked EGL functions - redirect to SDL */
static EGLDisplay hook_eglGetDisplay(EGLNativeDisplayType d) { return fake_display; }
static EGLBoolean hook_eglInitialize(EGLDisplay d, EGLint *maj, EGLint *min) { if(maj)*maj=1; if(min)*min=5; return EGL_TRUE; }
static EGLint hook_eglGetError(void) { return EGL_SUCCESS; }
static EGLBoolean hook_eglChooseConfig(EGLDisplay d, const EGLint *a, EGLConfig *c, EGLint cs, EGLint *nc) {
    if(c && cs>0) { c[0]=(EGLConfig)0x1; } if(nc) *nc=1; return EGL_TRUE;
}
static EGLBoolean hook_eglGetConfigAttrib(EGLDisplay d, EGLConfig c, EGLint a, EGLint *v) {
    if(!v) return EGL_FALSE;
    switch(a) { case EGL_RED_SIZE: case EGL_GREEN_SIZE: case EGL_BLUE_SIZE: *v=8; break; case EGL_ALPHA_SIZE: *v=0; break; case EGL_DEPTH_SIZE: *v=16; break; case EGL_STENCIL_SIZE: *v=0; break; case EGL_NATIVE_VISUAL_ID: *v=0; break; default: *v=0; }
    return EGL_TRUE;
}
static EGLSurface hook_eglCreateWindowSurface(EGLDisplay d, EGLConfig c, EGLNativeWindowType w, const EGLint *a) {
    debugPrintf("hook_eglCreateWindowSurface called\n"); return fake_surface;
}
static EGLBoolean hook_eglQuerySurface(EGLDisplay d, EGLSurface s, EGLint a, EGLint *v) {
    if(!v) return EGL_FALSE;
    if(a==EGL_WIDTH) *v=g_screen_w; else if(a==EGL_HEIGHT) *v=g_screen_h; else *v=0;
    return EGL_TRUE;
}
static EGLContext hook_eglCreateContext(EGLDisplay d, EGLConfig c, EGLContext sh, const EGLint *a) {
    debugPrintf("hook_eglCreateContext called\n"); return fake_context;
}
static EGLBoolean hook_eglDestroyContext(EGLDisplay d, EGLContext c) { return EGL_TRUE; }
static EGLBoolean hook_eglMakeCurrent(EGLDisplay d, EGLSurface dr, EGLSurface rd, EGLContext c) {
    SDL_GL_MakeCurrent(g_sdl_window, g_sdl_glctx);
    return EGL_TRUE;
}
static EGLBoolean hook_eglDestroySurface(EGLDisplay d, EGLSurface s) { return EGL_TRUE; }
static EGLBoolean hook_eglTerminate(EGLDisplay d) { return EGL_TRUE; }
static EGLBoolean hook_eglSwapBuffers(EGLDisplay d, EGLSurface s) {
    SDL_GL_SwapWindow(g_sdl_window);
    return EGL_TRUE;
}
static void *hook_eglGetProcAddress(const char *name) { return SDL_GL_GetProcAddress(name); }

/* ANativeWindow stub */
static int32_t fake_ANativeWindow_setBuffersGeometry(void *w, int32_t width, int32_t height, int32_t fmt) {
    debugPrintf("ANativeWindow_setBuffersGeometry: %dx%d fmt=%d\n", width, height, fmt);
    return 0;
}

/* OpenSL ES stub */
static int fake_slCreateEngine(void **eng, unsigned int nopt, void *opts, unsigned int ni, void *ids, void *req) {
    debugPrintf("slCreateEngine called (stub)\n");
    return -1; /* SL_RESULT_INTERNAL_ERROR - audio won't work but game won't crash */
}

/* Android stubs */
static int32_t fake_AConfig_getInt(void *c) { return 0; }
static void fake_AConfig_getLang(void *c, char *l) { l[0]='e'; l[1]='n'; }
static void fake_AConfig_getCountry(void *c, char *l) { l[0]='U'; l[1]='S'; }
static void *fake_AConfig_new(void) { return calloc(1,64); }
static void fake_AConfig_delete(void *c) { free(c); }
static void fake_AConfig_fromAM(void *c, void *am) { }
static void fake_ANativeActivity_finish(void *a) { debugPrintf("ANativeActivity_finish called\n"); }

/* ALooper - functional implementation */
#define MAX_LOOPER_FDS 8
static struct { int fd; int ident; int events; void *cb; void *data; } looper_fds[MAX_LOOPER_FDS];
static int looper_fd_count = 0;

static void *fake_ALooper_prepare(int opts) { looper_fd_count = 0; return (void*)0x100004; }

static int fake_ALooper_addFd(void *l, int fd, int ident, int events, void *cb, void *data) {
    if (looper_fd_count < MAX_LOOPER_FDS) {
        looper_fds[looper_fd_count].fd = fd;
        looper_fds[looper_fd_count].ident = ident;
        looper_fds[looper_fd_count].events = events;
        looper_fds[looper_fd_count].cb = cb;
        looper_fds[looper_fd_count].data = data;
        looper_fd_count++;
    }
    return 1;
}

static int fake_ALooper_pollOnce(int timeout_ms, int *outFd, int *outEvents, void **outData) {
    if (looper_fd_count == 0) { usleep(16000); return 0; }
    struct pollfd pfds[MAX_LOOPER_FDS];
    for (int i = 0; i < looper_fd_count; i++) {
        pfds[i].fd = looper_fds[i].fd;
        pfds[i].events = POLLIN;
        pfds[i].revents = 0;
    }
    int ret = poll(pfds, looper_fd_count, timeout_ms);
    if (ret > 0) {
        for (int i = 0; i < looper_fd_count; i++) {
            if (pfds[i].revents & POLLIN) {
                if (outFd) *outFd = looper_fds[i].fd;
                if (outEvents) *outEvents = pfds[i].revents;
                if (outData) *outData = looper_fds[i].data;
                return looper_fds[i].ident;
            }
        }
    }
    return 0; /* ALOOPER_POLL_TIMEOUT */
}

/* AInputQueue stubs */
static void fake_AIQ_attachLooper(void *q, void *l, int id, void *cb, void *d) { }
static void fake_AIQ_detachLooper(void *q) { }
static int fake_AIQ_getEvent(void *q, void **ev) { return -1; }
static int fake_AIQ_preDispatch(void *q, void *ev) { return 0; }
static void fake_AIQ_finishEvent(void *q, void *ev, int h) { }

/* AInputEvent stubs */
static int fake_AIE_getType(void *e) { return 0; }
static int fake_AIE_getSource(void *e) { return 0; }
static int fake_AIE_getDevId(void *e) { return 0; }
static int fake_AME_getAction(void *e) { return 0; }
static int fake_AME_getPtrCount(void *e) { return 0; }
static int fake_AKE_getAction(void *e) { return 0; }
static int fake_AKE_getKeyCode(void *e) { return 0; }
static int fake_AME_getPtrId(void *e, int i) { return 0; }
static float fake_AME_getX(void *e, int i) { return 0.0f; }
static float fake_AME_getY(void *e, int i) { return 0.0f; }
static int64_t fake_AME_getEvTime(void *e) { return 0; }
static int fake_AAsset_openFD(void *a, int64_t *s, int64_t *l) { return -1; }

/* OpenSL ES IID stubs */
static const int SL_IID_ENGINE_fake[4] = {0};
static const int SL_IID_VOLUME_fake[4] = {0};
static const int SL_IID_BUFFERQUEUE_fake[4] = {0};
static const int SL_IID_SEEK_fake[4] = {0};
static const int SL_IID_PLAY_fake[4] = {0};

/* Fortified function wrappers */
static size_t __strlen_chk_fake(const char *s, size_t maxlen) { return strlen(s); }
static char *__strncpy_chk_fake(char *d, const char *s, size_t n, size_t ds) { return strncpy(d,s,n); }
static char *__strncpy_chk2_fake(char *d, const char *s, size_t n, size_t ds, size_t ss) { return strncpy(d,s,n); }
static int __vsprintf_chk_fake(char *s, int f, size_t sz, const char *fmt, va_list ap) { return vsprintf(s,fmt,ap); }
static int __vsnprintf_chk_fake(char *s, size_t n, int f, size_t sz, const char *fmt, va_list ap) { return vsnprintf(s,n,fmt,ap); }
static char *__strchr_chk_fake(const char *s, int c, size_t n) { return strchr(s,c); }
static void *__memcpy_chk_fake(void *d, const void *s, size_t n, size_t ds) { return memcpy(d,s,n); }
static void *__memmove_chk_fake(void *d, const void *s, size_t n, size_t ds) { return memmove(d,s,n); }
static void *__memset_chk_fake(void *s, int c, size_t n, size_t ds) { return memset(s,c,n); }
static ssize_t __write_chk_fake(int fd, const void *b, size_t n, size_t bs) { return write(fd,b,n); }
static ssize_t __read_chk_fake(int fd, void *b, size_t n, size_t bs) { return read(fd,b,n); }
static char *__strcpy_chk_fake(char *d, const char *s, size_t ds) { return strcpy(d,s); }
static size_t __fwrite_chk_fake(const void *p, size_t s, size_t n, FILE *f) { return fwrite(p,s,n,f); }
static size_t __fread_chk_fake(void *p, size_t s, size_t n, FILE *f) { return fread(p,s,n,f); }
static void *__memchr_chk_fake(const void *s, int c, size_t n, size_t ns) { return memchr(s,c,n); }
static int __poll_chk_fake(struct pollfd *fds, nfds_t n, int to, size_t s) { return poll(fds,n,to); }
static int __open_2_fake(const char *p, int f) { return open(p,f); }
static void __FD_SET_chk_fake(int fd, fd_set *s, size_t n) { FD_SET(fd,s); }
static struct cmsghdr *__cmsg_nxthdr_fake(struct msghdr *m, struct cmsghdr *c) { return CMSG_NXTHDR(m,c); }

/* Android property stub */
static int __system_property_get_fake(const char *n, char *v) { v[0]=0; return 0; }
static void android_set_abort_message_fake(const char *m) { debugPrintf("abort: %s\n",m); }

/* sincos */
static void sincosf_fake(float x, float *s, float *c) { *s=sinf(x); *c=cosf(x); }

/* pthread extras */
static int pthread_mutex_trylock_fake(pthread_mutex_t **uid) {
    if (!uid || !*uid) return -1;
    return pthread_mutex_trylock(*uid);
}

/* ctype */
static size_t __ctype_get_mb_cur_max_fake(void) { return 4; }

/* stubs for unavailable locale functions */
static long long strtoll_l_fake(const char *s, char **e, int b, locale_t l) { return strtoll(s,e,b); }
static unsigned long long strtoull_l_fake(const char *s, char **e, int b, locale_t l) { return strtoull(s,e,b); }
static long double strtold_l_fake(const char *s, char **e, locale_t l) { return strtold(s,e); }

/* Extra import table */
DynLibFunction dynlib_functions_extra[] = {
    /* libc core */
    {"longjmp", (uintptr_t)&longjmp},
    {"stderr", (uintptr_t)&stderr},
    {"stdout", (uintptr_t)&stdout},
    {"stdin", (uintptr_t)&stdin},
    {"strdup", (uintptr_t)&strdup},
    {"pipe", (uintptr_t)&pipe},
    {"access", (uintptr_t)&access},
    {"tmpnam", (uintptr_t)&tmpnam},
    {"rename", (uintptr_t)&rename},
    {"remove", (uintptr_t)&remove},
    {"modf", (uintptr_t)&modf},
    {"frexp", (uintptr_t)&frexp},
    {"atan", (uintptr_t)&atan},
    {"sincosf", (uintptr_t)&sincosf_fake},
    {"localeconv", (uintptr_t)&localeconv},
    {"strptime", (uintptr_t)&strptime},
    {"mktime", (uintptr_t)&mktime},
    {"strspn", (uintptr_t)&strspn},
    {"strcspn", (uintptr_t)&strcspn},
    {"fileno", (uintptr_t)&fileno},
    {"vfprintf", (uintptr_t)&vfprintf},
    {"vasprintf", (uintptr_t)&vasprintf},
    {"vsscanf", (uintptr_t)&vsscanf},
    {"fseeko", (uintptr_t)&fseeko},
    {"ftello", (uintptr_t)&ftello},
    {"fstat", (uintptr_t)&fstat},
    {"unlink", (uintptr_t)&unlink},
    {"memrchr", (uintptr_t)&memrchr},
    {"basename", (uintptr_t)&basename},
    {"realpath", (uintptr_t)&realpath},
    {"fcntl", (uintptr_t)&fcntl},
    {"select", (uintptr_t)&select},
    {"ioctl", (uintptr_t)&ioctl},
    {"getpid", (uintptr_t)&getpid},
    {"geteuid", (uintptr_t)&geteuid},
    {"getpwuid_r", (uintptr_t)&getpwuid_r},
    {"getauxval", (uintptr_t)&getauxval},
    {"getentropy", (uintptr_t)&getentropy},
    {"sysconf", (uintptr_t)&sysconf},
    {"posix_memalign", (uintptr_t)&posix_memalign},
    {"sched_yield", (uintptr_t)&sched_yield},
    {"strerror_r", (uintptr_t)&strerror_r},
    {"swprintf", (uintptr_t)&swprintf},
    {"wmemchr", (uintptr_t)&wmemchr},
    {"wmemcmp", (uintptr_t)&wmemcmp},
    {"strtold", (uintptr_t)&strtold},
    {"strtoll", (uintptr_t)&strtoll},
    {"strtoull", (uintptr_t)&strtoull},
    {"wcstol", (uintptr_t)&wcstol},
    {"wcstoul", (uintptr_t)&wcstoul},
    {"wcstoll", (uintptr_t)&wcstoll},
    {"wcstoull", (uintptr_t)&wcstoull},
    {"wcstof", (uintptr_t)&wcstof},
    {"wcstod", (uintptr_t)&wcstod},
    {"wcstold", (uintptr_t)&wcstold},
    {"mbtowc", (uintptr_t)&mbtowc},
    {"mbrlen", (uintptr_t)&mbrlen},
    {"mbsrtowcs", (uintptr_t)&mbsrtowcs},
    {"mbsnrtowcs", (uintptr_t)&mbsnrtowcs},
    {"wcsnrtombs", (uintptr_t)&wcsnrtombs},

    /* mmap/memory */
    {"mmap", (uintptr_t)&mmap},
    {"mprotect", (uintptr_t)&mprotect},
    {"munmap", (uintptr_t)&munmap},
    {"madvise", (uintptr_t)&madvise},
    {"mlock", (uintptr_t)&mlock},
    {"syscall", (uintptr_t)&syscall},

    /* signal */
    {"signal", (uintptr_t)&signal},
    {"sigaction", (uintptr_t)&sigaction},
    {"sigemptyset", (uintptr_t)&sigemptyset},
    {"sigaltstack", (uintptr_t)&sigaltstack},

    /* directories */
    {"opendir", (uintptr_t)&opendir},
    {"readdir", (uintptr_t)&readdir},
    {"closedir", (uintptr_t)&closedir},
    {"gmtime_r", (uintptr_t)&gmtime_r},
    {"gmtime", (uintptr_t)&gmtime},

    /* locale */
    {"setlocale", (uintptr_t)&setlocale},
    {"newlocale", (uintptr_t)&newlocale},
    {"uselocale", (uintptr_t)&uselocale},
    {"freelocale", (uintptr_t)&freelocale},
    {"strftime_l", (uintptr_t)&strftime_l},
    {"strcoll_l", (uintptr_t)&strcoll_l},
    {"strxfrm_l", (uintptr_t)&strxfrm_l},
    {"wcscoll_l", (uintptr_t)&wcscoll_l},
    {"wcsxfrm_l", (uintptr_t)&wcsxfrm_l},
    {"strtoll_l", (uintptr_t)&strtoll_l_fake},
    {"strtoull_l", (uintptr_t)&strtoull_l_fake},
    {"strtold_l", (uintptr_t)&strtold_l_fake},
    {"iswlower_l", (uintptr_t)&iswlower_l},
    {"iswspace_l", (uintptr_t)&iswspace_l},
    {"iswprint_l", (uintptr_t)&iswprint_l},
    {"iswblank_l", (uintptr_t)&iswblank_l},
    {"iswcntrl_l", (uintptr_t)&iswcntrl_l},
    {"iswupper_l", (uintptr_t)&iswupper_l},
    {"iswalpha_l", (uintptr_t)&iswalpha_l},
    {"iswdigit_l", (uintptr_t)&iswdigit_l},
    {"iswpunct_l", (uintptr_t)&iswpunct_l},
    {"iswxdigit_l", (uintptr_t)&iswxdigit_l},
    {"towupper_l", (uintptr_t)&towupper_l},
    {"towlower_l", (uintptr_t)&towlower_l},

    /* pthread extras */
    {"pthread_attr_init", (uintptr_t)&pthread_attr_init},
    {"pthread_attr_setdetachstate", (uintptr_t)&pthread_attr_setdetachstate},
    {"pthread_attr_destroy", (uintptr_t)&pthread_attr_destroy},
    {"pthread_exit", (uintptr_t)&pthread_exit},
    {"pthread_detach", (uintptr_t)&pthread_detach},
    {"pthread_equal", (uintptr_t)&pthread_equal},
    {"pthread_mutex_trylock", (uintptr_t)&pthread_mutex_trylock_fake},
    {"pthread_rwlock_init", (uintptr_t)&pthread_rwlock_init},
    {"pthread_rwlock_rdlock", (uintptr_t)&pthread_rwlock_rdlock},
    {"pthread_rwlock_wrlock", (uintptr_t)&pthread_rwlock_wrlock},
    {"pthread_rwlock_unlock", (uintptr_t)&pthread_rwlock_unlock},
    {"pthread_rwlock_destroy", (uintptr_t)&pthread_rwlock_destroy},

    /* networking */
    {"socket", (uintptr_t)&socket},
    {"connect", (uintptr_t)&connect},
    {"bind", (uintptr_t)&bind},
    {"listen", (uintptr_t)&listen},
    {"accept", (uintptr_t)&accept},
    {"accept4", (uintptr_t)&accept4},
    {"shutdown", (uintptr_t)&shutdown},
    {"sendto", (uintptr_t)&sendto},
    {"recvfrom", (uintptr_t)&recvfrom},
    {"sendmmsg", (uintptr_t)&sendmmsg},
    {"recvmmsg", (uintptr_t)&recvmmsg},
    {"getsockopt", (uintptr_t)&getsockopt},
    {"setsockopt", (uintptr_t)&setsockopt},
    {"getsockname", (uintptr_t)&getsockname},
    {"getpeername", (uintptr_t)&getpeername},
    {"gethostbyname", (uintptr_t)&gethostbyname},
    {"getaddrinfo", (uintptr_t)&getaddrinfo},
    {"freeaddrinfo", (uintptr_t)&freeaddrinfo},
    {"getnameinfo", (uintptr_t)&getnameinfo},
    {"gai_strerror", (uintptr_t)&gai_strerror},
    {"inet_pton", (uintptr_t)&inet_pton},
    {"inet_ntop", (uintptr_t)&inet_ntop},
    {"getifaddrs", (uintptr_t)&getifaddrs},
    {"freeifaddrs", (uintptr_t)&freeifaddrs},
    {"if_nametoindex", (uintptr_t)&if_nametoindex},
    {"poll", (uintptr_t)&poll},
    {"eventfd", (uintptr_t)&eventfd},
    {"socketpair", (uintptr_t)&socketpair},

    /* C++ ABI */
    {"__cxa_finalize", (uintptr_t)&ret0},
    {"__cxa_atexit", (uintptr_t)&ret0},
    {"__register_atfork", (uintptr_t)&ret0},

    /* dl */
    {"dlerror", (uintptr_t)&dlerror},
    {"dl_iterate_phdr", (uintptr_t)&dl_iterate_phdr},

    /* syslog */
    {"openlog", (uintptr_t)&openlog},
    {"syslog", (uintptr_t)&syslog},
    {"closelog", (uintptr_t)&closelog},

    /* fortified */
    {"__strlen_chk", (uintptr_t)&__strlen_chk_fake},
    {"__strncpy_chk", (uintptr_t)&__strncpy_chk_fake},
    {"__strncpy_chk2", (uintptr_t)&__strncpy_chk2_fake},
    {"__vsprintf_chk", (uintptr_t)&__vsprintf_chk_fake},
    {"__vsnprintf_chk", (uintptr_t)&__vsnprintf_chk_fake},
    {"__strchr_chk", (uintptr_t)&__strchr_chk_fake},
    {"__memcpy_chk", (uintptr_t)&__memcpy_chk_fake},
    {"__memmove_chk", (uintptr_t)&__memmove_chk_fake},
    {"__memset_chk", (uintptr_t)&__memset_chk_fake},
    {"__write_chk", (uintptr_t)&__write_chk_fake},
    {"__read_chk", (uintptr_t)&__read_chk_fake},
    {"__strcpy_chk", (uintptr_t)&__strcpy_chk_fake},
    {"__fwrite_chk", (uintptr_t)&__fwrite_chk_fake},
    {"__fread_chk", (uintptr_t)&__fread_chk_fake},
    {"__memchr_chk", (uintptr_t)&__memchr_chk_fake},
    {"__poll_chk", (uintptr_t)&__poll_chk_fake},
    {"__open_2", (uintptr_t)&__open_2_fake},
    {"__FD_SET_chk", (uintptr_t)&__FD_SET_chk_fake},
    {"__cmsg_nxthdr", (uintptr_t)&__cmsg_nxthdr_fake},

    /* android system */
    {"__system_property_get", (uintptr_t)&__system_property_get_fake},
    {"android_set_abort_message", (uintptr_t)&android_set_abort_message_fake},
    {"__ctype_get_mb_cur_max", (uintptr_t)&__ctype_get_mb_cur_max_fake},

    /* Android NDK - AConfiguration */
    {"AConfiguration_new", (uintptr_t)&fake_AConfig_new},
    {"AConfiguration_delete", (uintptr_t)&fake_AConfig_delete},
    {"AConfiguration_fromAssetManager", (uintptr_t)&fake_AConfig_fromAM},
    {"AConfiguration_getDensity", (uintptr_t)&fake_AConfig_getInt},
    {"AConfiguration_getScreenLong", (uintptr_t)&fake_AConfig_getInt},
    {"AConfiguration_getScreenSize", (uintptr_t)&fake_AConfig_getInt},
    {"AConfiguration_getLanguage", (uintptr_t)&fake_AConfig_getLang},
    {"AConfiguration_getCountry", (uintptr_t)&fake_AConfig_getCountry},
    {"ANativeActivity_finish", (uintptr_t)&fake_ANativeActivity_finish},

    /* Android NDK - ALooper */
    {"ALooper_prepare", (uintptr_t)&fake_ALooper_prepare},
    {"ALooper_addFd", (uintptr_t)&fake_ALooper_addFd},
    {"ALooper_pollOnce", (uintptr_t)&fake_ALooper_pollOnce},

    /* Android NDK - AInputQueue */
    {"AInputQueue_attachLooper", (uintptr_t)&fake_AIQ_attachLooper},
    {"AInputQueue_detachLooper", (uintptr_t)&fake_AIQ_detachLooper},
    {"AInputQueue_getEvent", (uintptr_t)&fake_AIQ_getEvent},
    {"AInputQueue_preDispatchEvent", (uintptr_t)&fake_AIQ_preDispatch},
    {"AInputQueue_finishEvent", (uintptr_t)&fake_AIQ_finishEvent},

    /* Android NDK - AInputEvent/AMotionEvent/AKeyEvent */
    {"AInputEvent_getType", (uintptr_t)&fake_AIE_getType},
    {"AInputEvent_getSource", (uintptr_t)&fake_AIE_getSource},
    {"AInputEvent_getDeviceId", (uintptr_t)&fake_AIE_getDevId},
    {"AMotionEvent_getAction", (uintptr_t)&fake_AME_getAction},
    {"AMotionEvent_getPointerCount", (uintptr_t)&fake_AME_getPtrCount},
    {"AMotionEvent_getPointerId", (uintptr_t)&fake_AME_getPtrId},
    {"AMotionEvent_getX", (uintptr_t)&fake_AME_getX},
    {"AMotionEvent_getY", (uintptr_t)&fake_AME_getY},
    {"AMotionEvent_getEventTime", (uintptr_t)&fake_AME_getEvTime},
    {"AKeyEvent_getAction", (uintptr_t)&fake_AKE_getAction},
    {"AKeyEvent_getKeyCode", (uintptr_t)&fake_AKE_getKeyCode},
    {"AAsset_openFileDescriptor", (uintptr_t)&fake_AAsset_openFD},

    /* OpenSL ES IIDs */
    {"SL_IID_ENGINE", (uintptr_t)&SL_IID_ENGINE_fake},
    {"SL_IID_VOLUME", (uintptr_t)&SL_IID_VOLUME_fake},
    {"SL_IID_BUFFERQUEUE", (uintptr_t)&SL_IID_BUFFERQUEUE_fake},
    {"SL_IID_SEEK", (uintptr_t)&SL_IID_SEEK_fake},
    {"SL_IID_PLAY", (uintptr_t)&SL_IID_PLAY_fake},

    /* EGL hooks - return our pre-created context */
    {"eglGetDisplay", (uintptr_t)&hook_eglGetDisplay},
    {"eglInitialize", (uintptr_t)&hook_eglInitialize},
    {"eglGetError", (uintptr_t)&hook_eglGetError},
    {"eglChooseConfig", (uintptr_t)&hook_eglChooseConfig},
    {"eglGetConfigAttrib", (uintptr_t)&hook_eglGetConfigAttrib},
    {"eglCreateWindowSurface", (uintptr_t)&hook_eglCreateWindowSurface},
    {"eglQuerySurface", (uintptr_t)&hook_eglQuerySurface},
    {"eglCreateContext", (uintptr_t)&hook_eglCreateContext},
    {"eglDestroyContext", (uintptr_t)&hook_eglDestroyContext},
    {"eglMakeCurrent", (uintptr_t)&hook_eglMakeCurrent},
    {"eglDestroySurface", (uintptr_t)&hook_eglDestroySurface},
    {"eglTerminate", (uintptr_t)&hook_eglTerminate},
    {"eglSwapBuffers", (uintptr_t)&hook_eglSwapBuffers},
    {"eglGetProcAddress", (uintptr_t)&hook_eglGetProcAddress},

    /* ANativeWindow */
    {"ANativeWindow_setBuffersGeometry", (uintptr_t)&fake_ANativeWindow_setBuffersGeometry},

    /* GL extras */
    {"glIsTexture", (uintptr_t)&glIsTexture},
    {"glIsFramebuffer", (uintptr_t)&glIsFramebuffer},
    {"glIsRenderbuffer", (uintptr_t)&glIsRenderbuffer},
    {"glValidateProgram", (uintptr_t)&glValidateProgram},

    /* setjmp/longjmp */
    {"setjmp", (uintptr_t)&setjmp},
    {"longjmp", (uintptr_t)&longjmp},

    /* OpenSL ES */
    {"slCreateEngine", (uintptr_t)&fake_slCreateEngine},

    /* zlib extras */
    {"inflateInit_", (uintptr_t)&inflateInit_},
    {"inflateReset", (uintptr_t)&inflateReset},
    {"inflateReset2", (uintptr_t)&inflateReset2},
    {"deflateReset", (uintptr_t)&deflateReset},
};

size_t dynlib_numfunctions_extra = sizeof(dynlib_functions_extra) / sizeof(*dynlib_functions_extra);
