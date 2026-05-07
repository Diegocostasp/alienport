/*
 * imports.c -- Symbol import resolution for Android .so on Linux
 *
 * Based on Max Payne ARM64 port by Jaakko Lukkari.
 * Maps Android libc/libm/libGLESv2/libEGL/liblog symbols to Linux equivalents.
 */

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <semaphore.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <wchar.h>
#include <wctype.h>
#include <zlib.h>
#include <dlfcn.h>

#include "so_util.h"
#include "util.h"

/* Fake __sF for Android's stdio compatibility */
static uint8_t fake_sF[3][0x100];

static uint64_t __stack_chk_guard_fake = 0x4242424242424242;

FILE *stderr_fake = NULL;

int *__errno_location_fake(void) { return &errno; }

void __assert2(const char *file, int line, const char *func, const char *expr) {
    debugPrintf("assertion failed:\n%s:%d (%s): %s\n", file, line, func, expr);
    assert(0);
}

int __android_log_print(int prio, const char *tag, const char *fmt, ...) {
    va_list list;
    static char string[0x1000];
    va_start(list, fmt);
    vsnprintf(string, sizeof(string), fmt, list);
    va_end(list);
    debugPrintf("[%s] %s\n", tag, string);
    return 0;
}

int __android_log_write(int prio, const char *tag, const char *text) {
    debugPrintf("[%s] %s\n", tag, text);
    return 0;
}

int __android_log_vprint(int prio, const char *tag, const char *fmt, va_list ap) {
    debugPrintf("[%s] ", tag);
    vprintf(fmt, ap);
    printf("\n");
    return 0;
}

/* pthread wrappers (Android struct sizes differ from glibc) */
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
    return asset;
}

static int fake_AAsset_read(AAsset *asset, void *buf, size_t count) {
    if (!asset || !asset->fp) return -1;
    return fread(buf, 1, count, asset->fp);
}

static off_t fake_AAsset_seek(AAsset *asset, off_t offset, int whence) {
    if (!asset || !asset->fp) return -1;
    fseek(asset->fp, offset, whence);
    return ftell(asset->fp);
}

static void fake_AAsset_close(AAsset *asset) {
    if (!asset) return;
    if (asset->fp) fclose(asset->fp);
    free(asset);
}

static off_t fake_AAsset_getLength(AAsset *asset) {
    if (!asset) return 0;
    return asset->length;
}

static off_t fake_AAsset_getRemainingLength(AAsset *asset) {
    if (!asset || !asset->fp) return 0;
    long current = ftell(asset->fp);
    return asset->length - current;
}

static int fake_AAsset_openFileDescriptor(AAsset *asset, off_t *outStart, off_t *outLength) {
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
    {"__sF", (uintptr_t)&fake_sF},
    {"__android_log_print", (uintptr_t)__android_log_print},
    {"__android_log_write", (uintptr_t)__android_log_write},
    {"__android_log_vprint", (uintptr_t)__android_log_vprint},
    {"__errno", (uintptr_t)&__errno_location_fake},
    {"__stack_chk_fail", (uintptr_t)&abort},
    {"__stack_chk_guard", (uintptr_t)&__stack_chk_guard_fake},
    {"__assert2", (uintptr_t)&__assert2},

    /* AAssetManager stubs */
    {"AAssetManager_open", (uintptr_t)&fake_AAssetManager_open},
    {"AAssetManager_fromJava", (uintptr_t)&fake_AAssetManager_fromJava},
    {"AAsset_close", (uintptr_t)&fake_AAsset_close},
    {"AAsset_getLength", (uintptr_t)&fake_AAsset_getLength},
    {"AAsset_getRemainingLength", (uintptr_t)&fake_AAsset_getRemainingLength},
    {"AAsset_read", (uintptr_t)&fake_AAsset_read},
    {"AAsset_seek", (uintptr_t)&fake_AAsset_seek},
    {"AAsset_openFileDescriptor", (uintptr_t)&fake_AAsset_openFileDescriptor},

    /* ANativeWindow stubs */
    {"ANativeWindow_setBuffersGeometry", (uintptr_t)&ret0},
    {"ANativeWindow_getWidth", (uintptr_t)&ret0},
    {"ANativeWindow_getHeight", (uintptr_t)&ret0},

    /* OpenSL ES stubs */
    {"slCreateEngine", (uintptr_t)&slCreateEngine},

    /* pthread */
    {"pthread_key_create", (uintptr_t)&ret0},
    {"pthread_key_delete", (uintptr_t)&ret0},
    {"pthread_getspecific", (uintptr_t)&ret0},
    {"pthread_setspecific", (uintptr_t)&ret0},
    {"pthread_cond_broadcast", (uintptr_t)&pthread_cond_broadcast_fake},
    {"pthread_cond_destroy", (uintptr_t)&pthread_cond_destroy_fake},
    {"pthread_cond_init", (uintptr_t)&pthread_cond_init_fake},
    {"pthread_cond_signal", (uintptr_t)&pthread_cond_signal_fake},
    {"pthread_cond_timedwait", (uintptr_t)&pthread_cond_timedwait_fake},
    {"pthread_cond_wait", (uintptr_t)&pthread_cond_wait_fake},
    {"pthread_create", (uintptr_t)&pthread_create_fake},
    {"pthread_join", (uintptr_t)&pthread_join},
    {"pthread_self", (uintptr_t)&pthread_self},
    {"pthread_setschedparam", (uintptr_t)&ret0},
    {"pthread_mutexattr_init", (uintptr_t)&ret0},
    {"pthread_mutexattr_settype", (uintptr_t)&ret0},
    {"pthread_mutexattr_destroy", (uintptr_t)&ret0},
    {"pthread_mutex_destroy", (uintptr_t)&pthread_mutex_destroy_fake},
    {"pthread_mutex_init", (uintptr_t)&pthread_mutex_init_fake},
    {"pthread_mutex_lock", (uintptr_t)&pthread_mutex_lock_fake},
    {"pthread_mutex_unlock", (uintptr_t)&pthread_mutex_unlock_fake},
    {"pthread_once", (uintptr_t)&pthread_once_fake},
    {"sched_get_priority_min", (uintptr_t)&retm1},

    /* math */
    {"acos", (uintptr_t)&acos}, {"acosf", (uintptr_t)&acosf},
    {"asin", (uintptr_t)&asin}, {"asinf", (uintptr_t)&asinf},
    {"atan2", (uintptr_t)&atan2}, {"atan2f", (uintptr_t)&atan2f},
    {"atanf", (uintptr_t)&atanf}, {"ceil", (uintptr_t)&ceil},
    {"ceilf", (uintptr_t)&ceilf},
    {"cos", (uintptr_t)&cos}, {"cosf", (uintptr_t)&cosf},
    {"exp", (uintptr_t)&exp}, {"expf", (uintptr_t)&expf},
    {"floor", (uintptr_t)&floor}, {"floorf", (uintptr_t)&floorf},
    {"fmod", (uintptr_t)&fmod}, {"fmodf", (uintptr_t)&fmodf},
    {"log", (uintptr_t)&log}, {"logf", (uintptr_t)&logf},
    {"log10", (uintptr_t)&log10}, {"log10f", (uintptr_t)&log10f},
    {"pow", (uintptr_t)&pow}, {"powf", (uintptr_t)&powf},
    {"sin", (uintptr_t)&sin}, {"sinf", (uintptr_t)&sinf},
    {"tan", (uintptr_t)&tan}, {"tanf", (uintptr_t)&tanf},
    {"sqrt", (uintptr_t)&sqrt}, {"sqrtf", (uintptr_t)&sqrtf},
    {"round", (uintptr_t)&round}, {"roundf", (uintptr_t)&roundf},
    {"lround", (uintptr_t)&lround}, {"fabs", (uintptr_t)&fabs},
    {"fabsf", (uintptr_t)&fabsf},

    /* stdlib */
    {"atoi", (uintptr_t)&atoi}, {"atof", (uintptr_t)&atof},
    {"calloc", (uintptr_t)&calloc}, {"free", (uintptr_t)&free},
    {"malloc", (uintptr_t)&malloc}, {"realloc", (uintptr_t)&realloc},
    {"abort", (uintptr_t)&abort}, {"exit", (uintptr_t)&exit},
    {"getenv", (uintptr_t)&getenv}, {"setenv", (uintptr_t)&setenv},
    {"srand", (uintptr_t)&srand}, {"rand", (uintptr_t)&rand},
    {"qsort", (uintptr_t)&qsort}, {"bsearch", (uintptr_t)&bsearch},
    {"abs", (uintptr_t)&abs},
    {"strtol", (uintptr_t)&strtol}, {"strtoul", (uintptr_t)&strtoul},
    {"strtod", (uintptr_t)&strtod}, {"strtof", (uintptr_t)&strtof},

    /* string */
    {"memcmp", (uintptr_t)&memcmp}, {"memcpy", (uintptr_t)&memcpy},
    {"memmove", (uintptr_t)&memmove}, {"memset", (uintptr_t)&memset},
    {"memchr", (uintptr_t)&memchr},
    {"strcasecmp", (uintptr_t)&strcasecmp}, {"strcat", (uintptr_t)&strcat},
    {"strchr", (uintptr_t)&strchr}, {"strcmp", (uintptr_t)&strcmp},
    {"strcpy", (uintptr_t)&strcpy}, {"strerror", (uintptr_t)&strerror},
    {"strlen", (uintptr_t)&strlen}, {"strncasecmp", (uintptr_t)&strncasecmp},
    {"strncat", (uintptr_t)&strncat}, {"strncmp", (uintptr_t)&strncmp},
    {"strncpy", (uintptr_t)&strncpy}, {"strrchr", (uintptr_t)&strrchr},
    {"strstr", (uintptr_t)&strstr}, {"strtok", (uintptr_t)&strtok},
    {"strpbrk", (uintptr_t)&strpbrk},

    /* stdio */
    {"printf", (uintptr_t)&debugPrintf},
    {"fprintf", (uintptr_t)&fprintf},
    {"sprintf", (uintptr_t)&sprintf}, {"snprintf", (uintptr_t)&snprintf},
    {"vsnprintf", (uintptr_t)&vsnprintf}, {"vsprintf", (uintptr_t)&vsprintf},
    {"sscanf", (uintptr_t)&sscanf},
    {"fopen", (uintptr_t)&fopen}, {"fclose", (uintptr_t)&fclose},
    {"fread", (uintptr_t)&fread}, {"fwrite", (uintptr_t)&fwrite},
    {"fseek", (uintptr_t)&fseek}, {"ftell", (uintptr_t)&ftell},
    {"fflush", (uintptr_t)&fflush}, {"feof", (uintptr_t)&feof},
    {"ferror", (uintptr_t)&ferror}, {"fgets", (uintptr_t)&fgets},
    {"fputs", (uintptr_t)&fputs}, {"fputc", (uintptr_t)&fputc},
    {"fgetc", (uintptr_t)&fgetc}, {"setvbuf", (uintptr_t)&setvbuf},
    {"fdopen", (uintptr_t)&fdopen},

    /* file I/O */
    {"close", (uintptr_t)&close}, {"lseek", (uintptr_t)&lseek},
    {"mkdir", (uintptr_t)&mkdir}, {"open", (uintptr_t)&open},
    {"read", (uintptr_t)&read}, {"write", (uintptr_t)&write},
    {"stat", (uintptr_t)stat},

    /* ctype */
    {"isspace", (uintptr_t)&isspace}, {"isdigit", (uintptr_t)&isdigit},
    {"isalpha", (uintptr_t)&isalpha}, {"isalnum", (uintptr_t)&isalnum},
    {"isupper", (uintptr_t)&isupper}, {"islower", (uintptr_t)&islower},
    {"tolower", (uintptr_t)&tolower}, {"toupper", (uintptr_t)&toupper},

    /* time */
    {"clock_gettime", (uintptr_t)&clock_gettime},
    {"gettimeofday", (uintptr_t)&gettimeofday},
    {"time", (uintptr_t)&time}, {"localtime", (uintptr_t)&localtime},
    {"localtime_r", (uintptr_t)&localtime_r},
    {"nanosleep", (uintptr_t)&nanosleep}, {"usleep", (uintptr_t)&usleep},
    {"strftime", (uintptr_t)&strftime},

    /* zlib */
    {"compress", (uintptr_t)&compress}, {"compress2", (uintptr_t)&compress2},
    {"uncompress", (uintptr_t)&uncompress},
    {"deflate", (uintptr_t)&deflate}, {"deflateEnd", (uintptr_t)&deflateEnd},
    {"deflateInit2_", (uintptr_t)&deflateInit2_},
    {"inflate", (uintptr_t)&inflate}, {"inflateEnd", (uintptr_t)&inflateEnd},
    {"inflateInit2_", (uintptr_t)&inflateInit2_},
    {"crc32", (uintptr_t)&crc32},

    /* dlopen/dlsym stubs moved to imports_extra.c */

    /* EGL - hooks are in imports_extra.c */
    {"eglQueryString", (uintptr_t)&eglQueryString},

    /* OpenGL ES 2.0 */
    {"glActiveTexture", (uintptr_t)&glActiveTexture},
    {"glAttachShader", (uintptr_t)&glAttachShader},
    {"glBindAttribLocation", (uintptr_t)&glBindAttribLocation},
    {"glBindBuffer", (uintptr_t)&glBindBuffer},
    {"glBindFramebuffer", (uintptr_t)&glBindFramebuffer},
    {"glBindRenderbuffer", (uintptr_t)&glBindRenderbuffer},
    {"glBindTexture", (uintptr_t)&glBindTexture},
    {"glBlendFunc", (uintptr_t)&glBlendFunc},
    {"glBlendFuncSeparate", (uintptr_t)&glBlendFuncSeparate},
    {"glBufferData", (uintptr_t)&glBufferData},
    {"glBufferSubData", (uintptr_t)&glBufferSubData},
    {"glCheckFramebufferStatus", (uintptr_t)&glCheckFramebufferStatus},
    {"glClear", (uintptr_t)&glClear},
    {"glClearColor", (uintptr_t)&glClearColor},
    {"glClearDepthf", (uintptr_t)&glClearDepthf},
    {"glClearStencil", (uintptr_t)&glClearStencil},
    {"glColorMask", (uintptr_t)&glColorMask},
    {"glCompileShader", (uintptr_t)&glCompileShader},
    {"glCompressedTexImage2D", (uintptr_t)&glCompressedTexImage2D},
    {"glCreateProgram", (uintptr_t)&glCreateProgram},
    {"glCreateShader", (uintptr_t)&glCreateShader},
    {"glCullFace", (uintptr_t)&glCullFace},
    {"glDeleteBuffers", (uintptr_t)&glDeleteBuffers},
    {"glDeleteFramebuffers", (uintptr_t)&glDeleteFramebuffers},
    {"glDeleteProgram", (uintptr_t)&glDeleteProgram},
    {"glDeleteRenderbuffers", (uintptr_t)&glDeleteRenderbuffers},
    {"glDeleteShader", (uintptr_t)&glDeleteShader},
    {"glDeleteTextures", (uintptr_t)&glDeleteTextures},
    {"glDepthFunc", (uintptr_t)&glDepthFunc},
    {"glDepthMask", (uintptr_t)&glDepthMask},
    {"glDepthRangef", (uintptr_t)&glDepthRangef},
    {"glDisable", (uintptr_t)&glDisable},
    {"glDisableVertexAttribArray", (uintptr_t)&glDisableVertexAttribArray},
    {"glDrawArrays", (uintptr_t)&glDrawArrays},
    {"glDrawElements", (uintptr_t)&glDrawElements},
    {"glEnable", (uintptr_t)&glEnable},
    {"glEnableVertexAttribArray", (uintptr_t)&glEnableVertexAttribArray},
    {"glFinish", (uintptr_t)&glFinish},
    {"glFlush", (uintptr_t)&glFlush},
    {"glFramebufferRenderbuffer", (uintptr_t)&glFramebufferRenderbuffer},
    {"glFramebufferTexture2D", (uintptr_t)&glFramebufferTexture2D},
    {"glFrontFace", (uintptr_t)&glFrontFace},
    {"glGenBuffers", (uintptr_t)&glGenBuffers},
    {"glGenFramebuffers", (uintptr_t)&glGenFramebuffers},
    {"glGenRenderbuffers", (uintptr_t)&glGenRenderbuffers},
    {"glGenTextures", (uintptr_t)&glGenTextures},
    {"glGenerateMipmap", (uintptr_t)&glGenerateMipmap},
    {"glGetAttribLocation", (uintptr_t)&glGetAttribLocation},
    {"glGetError", (uintptr_t)&glGetError},
    {"glGetIntegerv", (uintptr_t)&glGetIntegerv},
    {"glGetProgramInfoLog", (uintptr_t)&glGetProgramInfoLog},
    {"glGetProgramiv", (uintptr_t)&glGetProgramiv},
    {"glGetShaderInfoLog", (uintptr_t)&glGetShaderInfoLog},
    {"glGetShaderiv", (uintptr_t)&glGetShaderiv},
    {"glGetString", (uintptr_t)&glGetString},
    {"glGetUniformLocation", (uintptr_t)&glGetUniformLocation},
    {"glHint", (uintptr_t)&glHint},
    {"glLineWidth", (uintptr_t)&glLineWidth},
    {"glLinkProgram", (uintptr_t)&glLinkProgram},
    {"glPixelStorei", (uintptr_t)&glPixelStorei},
    {"glPolygonOffset", (uintptr_t)&glPolygonOffset},
    {"glReadPixels", (uintptr_t)&glReadPixels},
    {"glRenderbufferStorage", (uintptr_t)&glRenderbufferStorage},
    {"glScissor", (uintptr_t)&glScissor},
    {"glShaderSource", (uintptr_t)&glShaderSource},
    {"glStencilFunc", (uintptr_t)&glStencilFunc},
    {"glStencilMask", (uintptr_t)&glStencilMask},
    {"glStencilOp", (uintptr_t)&glStencilOp},
    {"glTexImage2D", (uintptr_t)&glTexImage2D},
    {"glTexParameterf", (uintptr_t)&glTexParameterf},
    {"glTexParameteri", (uintptr_t)&glTexParameteri},
    {"glTexSubImage2D", (uintptr_t)&glTexSubImage2D},
    {"glUniform1f", (uintptr_t)&glUniform1f},
    {"glUniform1fv", (uintptr_t)&glUniform1fv},
    {"glUniform1i", (uintptr_t)&glUniform1i},
    {"glUniform2f", (uintptr_t)&glUniform2f},
    {"glUniform2fv", (uintptr_t)&glUniform2fv},
    {"glUniform3f", (uintptr_t)&glUniform3f},
    {"glUniform3fv", (uintptr_t)&glUniform3fv},
    {"glUniform4f", (uintptr_t)&glUniform4f},
    {"glUniform4fv", (uintptr_t)&glUniform4fv},
    {"glUniformMatrix3fv", (uintptr_t)&glUniformMatrix3fv},
    {"glUniformMatrix4fv", (uintptr_t)&glUniformMatrix4fv},
    {"glUseProgram", (uintptr_t)&glUseProgram},
    {"glVertexAttrib4fv", (uintptr_t)&glVertexAttrib4fv},
    {"glVertexAttribPointer", (uintptr_t)&glVertexAttribPointer},
    {"glViewport", (uintptr_t)&glViewport},

    /* setjmp */
    {"setjmp", (uintptr_t)&setjmp},

    /* wchar */
    {"wctob", (uintptr_t)&wctob}, {"wctype", (uintptr_t)&wctype},
    {"iswctype", (uintptr_t)&iswctype},
    {"towlower", (uintptr_t)&towlower}, {"towupper", (uintptr_t)&towupper},
    {"mbrtowc", (uintptr_t)&mbrtowc}, {"wcrtomb", (uintptr_t)&wcrtomb},
    {"wcslen", (uintptr_t)&wcslen}, {"btowc", (uintptr_t)&btowc},
};

size_t dynlib_numfunctions = sizeof(dynlib_functions) / sizeof(*dynlib_functions);
