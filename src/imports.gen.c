// imports.gen.c — GERADO by python script

#include "imports.h"

#include "so_util.h"

#include <stdio.h>

#include <stdarg.h>
int __android_log_print(int prio, const char* tag, const char* fmt, ...);
int __android_log_vprint(int prio, const char* tag, const char* fmt, va_list ap);
int __android_log_write(int prio, const char* tag, const char* text);
int compress2(void);
void *eglQueryString_shim(void *dpy, int name);


#include <zlib.h>
extern void *__stack_chk_guard;
void __stack_chk_fail(void);
// GLES prototypes to fix undeclared functions
#include <GLES2/gl2.h>


#include <stdint.h>
#include <assert.h>
#include <errno.h>

static uint8_t fake_sF[3][0x100];

void __assert2(const char *file, int line, const char *func, const char *expr) {
    printf("assertion failed:\n%s:%d (%s): %s\n", file, line, func, expr);
    assert(0);
}

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

  {"__android_log_print", (uintptr_t)&__android_log_print},  // liblog

  {"__android_log_vprint", (uintptr_t)&__android_log_vprint},  // liblog

  {"__android_log_write", (uintptr_t)&__android_log_write},  // liblog

  {"__assert2", (uintptr_t)&__assert2},
  {"__errno", (uintptr_t)&__errno},  // pass

  {"__sF", (uintptr_t)&fake_sF},
  {"__stack_chk_fail", (uintptr_t)&__stack_chk_fail},  // abi

  {"__stack_chk_guard", (uintptr_t)&__stack_chk_guard},  // abi

  {"abort", (uintptr_t)&abort},  // pass

  {"abs", (uintptr_t)&abs},  // pass

  {"acos", (uintptr_t)&acos},  // pass

  {"acosf", (uintptr_t)&acosf},
  {"asin", (uintptr_t)&asin},  // pass

  {"asinf", (uintptr_t)&asinf},
  {"atan2", (uintptr_t)&atan2},  // pass

  {"atan2f", (uintptr_t)&atan2f},  // pass

  {"atanf", (uintptr_t)&atanf},
  {"atof", (uintptr_t)&atof},  // pass

  {"atoi", (uintptr_t)&atoi},  // pass

  {"bsearch", (uintptr_t)&bsearch},  // pass

  {"btowc", (uintptr_t)&btowc},
  {"calloc", (uintptr_t)&calloc},  // pass

  {"ceil", (uintptr_t)&ceil},  // pass

  {"ceilf", (uintptr_t)&ceilf},  // pass

  {"clock_gettime", (uintptr_t)&clock_gettime},  // pass

  {"close", (uintptr_t)&close},  // pass

  {"compress", (uintptr_t)&compress},  // pass

  {"compress2", (uintptr_t)&compress2},  // pass

  {"cos", (uintptr_t)&cos},  // pass

  {"cosf", (uintptr_t)&cosf},  // pass

  {"crc32", (uintptr_t)&crc32},  // pass

  {"deflate", (uintptr_t)&deflate},  // pass

  {"deflateEnd", (uintptr_t)&deflateEnd},  // pass

  {"deflateInit2_", (uintptr_t)&deflateInit2_},  // pass

  {"eglQueryString", (uintptr_t)&eglQueryString_shim},  // egl_shim

  {"exit", (uintptr_t)&exit},  // pass

  {"exp", (uintptr_t)&exp},  // pass

  {"expf", (uintptr_t)&expf},  // pass

  {"fabs", (uintptr_t)&fabs},  // pass

  {"fabsf", (uintptr_t)&fabsf},  // pass

  {"fclose", (uintptr_t)&fclose},  // pass

  {"fdopen", (uintptr_t)&fdopen},  // pass

  {"feof", (uintptr_t)&feof},
  {"ferror", (uintptr_t)&ferror},
  {"fflush", (uintptr_t)&fflush},  // pass

  {"fgetc", (uintptr_t)&fgetc},  // pass

  {"fgets", (uintptr_t)&fgets},  // pass

  {"floor", (uintptr_t)&floor},  // pass

  {"floorf", (uintptr_t)&floorf},  // pass

  {"fmod", (uintptr_t)&fmod},  // pass

  {"fmodf", (uintptr_t)&fmodf},  // pass

  {"fopen", (uintptr_t)&fopen},  // pass

  {"fprintf", (uintptr_t)&fprintf},  // pass

  {"fputc", (uintptr_t)&fputc},  // pass

  {"fputs", (uintptr_t)&fputs},  // pass

  {"fread", (uintptr_t)&fread},  // pass

  {"free", (uintptr_t)&free},  // pass

  {"fseek", (uintptr_t)&fseek},  // pass

  {"ftell", (uintptr_t)&ftell},  // pass

  {"fwrite", (uintptr_t)&fwrite},  // pass

  {"getenv", (uintptr_t)&getenv},  // pass

  {"gettimeofday", (uintptr_t)&gettimeofday},  // pass

  {"glActiveTexture", (uintptr_t)&glActiveTexture},  // gles

  {"glAttachShader", (uintptr_t)&glAttachShader},  // gles

  {"glBindAttribLocation", (uintptr_t)&glBindAttribLocation},  // gles

  {"glBindBuffer", (uintptr_t)&glBindBuffer},  // gles

  {"glBindFramebuffer", (uintptr_t)&glBindFramebuffer},  // gles

  {"glBindRenderbuffer", (uintptr_t)&glBindRenderbuffer},  // gles

  {"glBindTexture", (uintptr_t)&glBindTexture},  // gles

  {"glBlendFunc", (uintptr_t)&glBlendFunc},  // gles

  {"glBlendFuncSeparate", (uintptr_t)&glBlendFuncSeparate},  // gles

  {"glBufferData", (uintptr_t)&glBufferData},  // gles

  {"glBufferSubData", (uintptr_t)&glBufferSubData},  // gles

  {"glCheckFramebufferStatus", (uintptr_t)&glCheckFramebufferStatus},  // gles

  {"glClear", (uintptr_t)&glClear},  // gles

  {"glClearColor", (uintptr_t)&glClearColor},  // gles

  {"glClearDepthf", (uintptr_t)&glClearDepthf},  // gles

  {"glClearStencil", (uintptr_t)&glClearStencil},  // gles

  {"glColorMask", (uintptr_t)&glColorMask},  // gles

  {"glCompileShader", (uintptr_t)&glCompileShader},  // gles

  {"glCompressedTexImage2D", (uintptr_t)&glCompressedTexImage2D},  // gles

  {"glCreateProgram", (uintptr_t)&glCreateProgram},  // gles

  {"glCreateShader", (uintptr_t)&glCreateShader},  // gles

  {"glCullFace", (uintptr_t)&glCullFace},  // gles

  {"glDeleteBuffers", (uintptr_t)&glDeleteBuffers},  // gles

  {"glDeleteFramebuffers", (uintptr_t)&glDeleteFramebuffers},  // gles

  {"glDeleteProgram", (uintptr_t)&glDeleteProgram},  // gles

  {"glDeleteRenderbuffers", (uintptr_t)&glDeleteRenderbuffers},  // gles

  {"glDeleteShader", (uintptr_t)&glDeleteShader},  // gles

  {"glDeleteTextures", (uintptr_t)&glDeleteTextures},  // gles

  {"glDepthFunc", (uintptr_t)&glDepthFunc},  // gles

  {"glDepthMask", (uintptr_t)&glDepthMask},  // gles

  {"glDepthRangef", (uintptr_t)&glDepthRangef},  // gles

  {"glDisable", (uintptr_t)&glDisable},  // gles

  {"glDisableVertexAttribArray", (uintptr_t)&glDisableVertexAttribArray},  // gles

  {"glDrawArrays", (uintptr_t)&glDrawArrays},  // gles

  {"glDrawElements", (uintptr_t)&glDrawElements},  // gles

  {"glEnable", (uintptr_t)&glEnable},  // gles

  {"glEnableVertexAttribArray", (uintptr_t)&glEnableVertexAttribArray},  // gles

  {"glFinish", (uintptr_t)&glFinish},  // gles

  {"glFlush", (uintptr_t)&glFlush},  // gles

  {"glFramebufferRenderbuffer", (uintptr_t)&glFramebufferRenderbuffer},  // gles

  {"glFramebufferTexture2D", (uintptr_t)&glFramebufferTexture2D},  // gles

  {"glFrontFace", (uintptr_t)&glFrontFace},  // gles

  {"glGenBuffers", (uintptr_t)&glGenBuffers},  // gles

  {"glGenFramebuffers", (uintptr_t)&glGenFramebuffers},  // gles

  {"glGenRenderbuffers", (uintptr_t)&glGenRenderbuffers},  // gles

  {"glGenTextures", (uintptr_t)&glGenTextures},  // gles

  {"glGenerateMipmap", (uintptr_t)&glGenerateMipmap},  // gles

  {"glGetAttribLocation", (uintptr_t)&glGetAttribLocation},  // gles

  {"glGetError", (uintptr_t)&glGetError},  // gles

  {"glGetIntegerv", (uintptr_t)&glGetIntegerv},  // gles

  {"glGetProgramInfoLog", (uintptr_t)&glGetProgramInfoLog},  // gles

  {"glGetProgramiv", (uintptr_t)&glGetProgramiv},  // gles

  {"glGetShaderInfoLog", (uintptr_t)&glGetShaderInfoLog},  // gles

  {"glGetShaderiv", (uintptr_t)&glGetShaderiv},  // gles

  {"glGetString", (uintptr_t)&glGetString},  // gles

  {"glGetUniformLocation", (uintptr_t)&glGetUniformLocation},  // gles

  {"glHint", (uintptr_t)&glHint},  // gles

  {"glLineWidth", (uintptr_t)&glLineWidth},  // gles

  {"glLinkProgram", (uintptr_t)&glLinkProgram},  // gles

  {"glPixelStorei", (uintptr_t)&glPixelStorei},  // gles

  {"glPolygonOffset", (uintptr_t)&glPolygonOffset},  // gles

  {"glReadPixels", (uintptr_t)&glReadPixels},  // gles

  {"glRenderbufferStorage", (uintptr_t)&glRenderbufferStorage},  // gles

  {"glScissor", (uintptr_t)&glScissor},  // gles

  {"glShaderSource", (uintptr_t)&glShaderSource},  // gles

  {"glStencilFunc", (uintptr_t)&glStencilFunc},  // gles

  {"glStencilMask", (uintptr_t)&glStencilMask},  // gles

  {"glStencilOp", (uintptr_t)&glStencilOp},  // gles

  {"glTexImage2D", (uintptr_t)&glTexImage2D},  // gles

  {"glTexParameterf", (uintptr_t)&glTexParameterf},  // gles

  {"glTexParameteri", (uintptr_t)&glTexParameteri},  // gles

  {"glTexSubImage2D", (uintptr_t)&glTexSubImage2D},  // gles

  {"glUniform1f", (uintptr_t)&glUniform1f},  // gles

  {"glUniform1fv", (uintptr_t)&glUniform1fv},  // gles

  {"glUniform1i", (uintptr_t)&glUniform1i},  // gles

  {"glUniform2f", (uintptr_t)&glUniform2f},  // gles

  {"glUniform2fv", (uintptr_t)&glUniform2fv},  // gles

  {"glUniform3f", (uintptr_t)&glUniform3f},  // gles

  {"glUniform3fv", (uintptr_t)&glUniform3fv},  // gles

  {"glUniform4f", (uintptr_t)&glUniform4f},  // gles

  {"glUniform4fv", (uintptr_t)&glUniform4fv},  // gles

  {"glUniformMatrix3fv", (uintptr_t)&glUniformMatrix3fv},  // gles

  {"glUniformMatrix4fv", (uintptr_t)&glUniformMatrix4fv},  // gles

  {"glUseProgram", (uintptr_t)&glUseProgram},  // gles

  {"glVertexAttrib4fv", (uintptr_t)&glVertexAttrib4fv},  // gles

  {"glVertexAttribPointer", (uintptr_t)&glVertexAttribPointer},  // gles

  {"glViewport", (uintptr_t)&glViewport},  // gles

  {"inflate", (uintptr_t)&inflate},  // pass

  {"inflateEnd", (uintptr_t)&inflateEnd},  // pass

  {"inflateInit2_", (uintptr_t)&inflateInit2_},  // pass

  {"isalnum", (uintptr_t)&isalnum},  // pass

  {"isalpha", (uintptr_t)&isalpha},  // pass

  {"isdigit", (uintptr_t)&isdigit},  // pass

  {"islower", (uintptr_t)&islower},  // pass

  {"isspace", (uintptr_t)&isspace},  // pass

  {"isupper", (uintptr_t)&isupper},  // pass

  {"iswctype", (uintptr_t)&iswctype},
  {"localtime", (uintptr_t)&localtime},  // pass

  {"localtime_r", (uintptr_t)&localtime_r},
  {"log", (uintptr_t)&log},  // pass

  {"log10", (uintptr_t)&log10},  // pass

  {"log10f", (uintptr_t)&log10f},
  {"logf", (uintptr_t)&logf},  // pass

  {"lround", (uintptr_t)&lround},
  {"lseek", (uintptr_t)&lseek},  // pass

  {"malloc", (uintptr_t)&malloc},  // pass

  {"mbrtowc", (uintptr_t)&mbrtowc},
  {"memchr", (uintptr_t)&memchr},  // pass

  {"memcmp", (uintptr_t)&memcmp},  // pass

  {"memcpy", (uintptr_t)&memcpy},  // pass

  {"memmove", (uintptr_t)&memmove},  // pass

  {"memset", (uintptr_t)&memset},  // pass

  {"mkdir", (uintptr_t)&mkdir},  // pass

  {"nanosleep", (uintptr_t)&nanosleep},  // pass

  {"open", (uintptr_t)&open},  // pass

  {"pow", (uintptr_t)&pow},  // pass

  {"powf", (uintptr_t)&powf},  // pass

  {"printf", (uintptr_t)&printf},  // pass

  {"pthread_cond_broadcast", (uintptr_t)&pthread_cond_broadcast_fake},  // pthread wrapper

  {"pthread_cond_destroy", (uintptr_t)&pthread_cond_destroy_fake},  // pthread wrapper

  {"pthread_cond_init", (uintptr_t)&pthread_cond_init_fake},  // pthread wrapper

  {"pthread_cond_signal", (uintptr_t)&pthread_cond_signal_fake},  // pthread wrapper

  {"pthread_cond_timedwait", (uintptr_t)&pthread_cond_timedwait_fake},  // pthread wrapper

  {"pthread_cond_wait", (uintptr_t)&pthread_cond_wait_fake},  // pthread wrapper

  {"pthread_create", (uintptr_t)&pthread_create_fake},  // pthread wrapper

  {"pthread_getspecific", (uintptr_t)&pthread_getspecific_fake},  // pthread wrapper

  {"pthread_join", (uintptr_t)&pthread_join_fake},  // pthread wrapper

  {"pthread_key_create", (uintptr_t)&pthread_key_create_fake},  // pthread wrapper

  {"pthread_key_delete", (uintptr_t)&pthread_key_delete_fake},  // pthread wrapper

  {"pthread_mutex_destroy", (uintptr_t)&pthread_mutex_destroy_fake},  // pthread wrapper

  {"pthread_mutex_init", (uintptr_t)&pthread_mutex_init_fake},  // pthread wrapper

  {"pthread_mutex_lock", (uintptr_t)&pthread_mutex_lock_fake},  // pthread wrapper

  {"pthread_mutex_unlock", (uintptr_t)&pthread_mutex_unlock_fake},  // pthread wrapper

  {"pthread_mutexattr_destroy", (uintptr_t)&pthread_mutexattr_destroy_fake},  // pthread wrapper

  {"pthread_mutexattr_init", (uintptr_t)&pthread_mutexattr_init_fake},  // pthread wrapper

  {"pthread_mutexattr_settype", (uintptr_t)&pthread_mutexattr_settype_fake},  // pthread wrapper

  {"pthread_once", (uintptr_t)&pthread_once_fake},  // pthread wrapper

  {"pthread_self", (uintptr_t)&pthread_self_fake},  // pthread wrapper

  {"pthread_setschedparam", (uintptr_t)&pthread_setschedparam_fake},  // pthread wrapper

  {"pthread_setspecific", (uintptr_t)&pthread_setspecific_fake},  // pthread wrapper

  {"qsort", (uintptr_t)&qsort},  // pass

  {"rand", (uintptr_t)&rand},  // pass

  {"read", (uintptr_t)&read},  // pass

  {"realloc", (uintptr_t)&realloc},  // pass

  {"round", (uintptr_t)&round},  // pass

  {"roundf", (uintptr_t)&roundf},  // pass

  {"sched_get_priority_min", (uintptr_t)&sched_get_priority_min_fake},
  {"setenv", (uintptr_t)&setenv},  // pass

  {"setjmp", (uintptr_t)&setjmp},
  {"setvbuf", (uintptr_t)&setvbuf},  // pass

  {"sin", (uintptr_t)&sin},  // pass

  {"sinf", (uintptr_t)&sinf},  // pass

  {"slCreateEngine", (uintptr_t)&slCreateEngine_shim},  // opensles_shim

  {"snprintf", (uintptr_t)&snprintf},  // pass

  {"sprintf", (uintptr_t)&sprintf},  // pass

  {"sqrt", (uintptr_t)&sqrt},  // pass

  {"sqrtf", (uintptr_t)&sqrtf},  // pass

  {"srand", (uintptr_t)&srand},  // pass

  {"sscanf", (uintptr_t)&sscanf},  // pass

  {"stat", (uintptr_t)&stat},  // pass

  {"strcasecmp", (uintptr_t)&strcasecmp},  // pass

  {"strcat", (uintptr_t)&strcat},  // pass

  {"strchr", (uintptr_t)&strchr},  // pass

  {"strcmp", (uintptr_t)&strcmp},  // pass

  {"strcpy", (uintptr_t)&strcpy},  // pass

  {"strerror", (uintptr_t)&strerror},  // pass

  {"strftime", (uintptr_t)&strftime},  // pass

  {"strlen", (uintptr_t)&strlen},  // pass

  {"strncasecmp", (uintptr_t)&strncasecmp},  // pass

  {"strncat", (uintptr_t)&strncat},  // pass

  {"strncmp", (uintptr_t)&strncmp},  // pass

  {"strncpy", (uintptr_t)&strncpy},  // pass

  {"strpbrk", (uintptr_t)&strpbrk},
  {"strrchr", (uintptr_t)&strrchr},  // pass

  {"strstr", (uintptr_t)&strstr},  // pass

  {"strtod", (uintptr_t)&strtod},  // pass

  {"strtof", (uintptr_t)&strtof},  // pass

  {"strtok", (uintptr_t)&strtok},  // pass

  {"strtol", (uintptr_t)&strtol},  // pass

  {"strtoul", (uintptr_t)&strtoul},  // pass

  {"tan", (uintptr_t)&tan},  // pass

  {"tanf", (uintptr_t)&tanf},  // pass

  {"time", (uintptr_t)&time},  // pass

  {"tolower", (uintptr_t)&tolower},  // pass

  {"toupper", (uintptr_t)&toupper},  // pass

  {"towlower", (uintptr_t)&towlower},
  {"towupper", (uintptr_t)&towupper},
  {"uncompress", (uintptr_t)&uncompress},  // pass

  {"usleep", (uintptr_t)&usleep},  // pass

  {"vsnprintf", (uintptr_t)&vsnprintf},  // pass

  {"vsprintf", (uintptr_t)&vsprintf},  // pass

  {"wcrtomb", (uintptr_t)&wcrtomb},
  {"wcslen", (uintptr_t)&wcslen},
  {"wctob", (uintptr_t)&wctob},
  {"wctype", (uintptr_t)&wctype},
  {"write", (uintptr_t)&write},  // pass

};

const int dynlib_functions_count = sizeof(dynlib_functions)/sizeof(dynlib_functions[0]);
