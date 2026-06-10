#include "imports.h"
#include "so_util.h"
#include "egl_shim.h"
#include <libgen.h>\n#include <sys/auxv.h>\n#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <GLES2/gl2.h>
#include <signal.h>
#include <zlib.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <syslog.h>
#include <dlfcn.h>
#include <setjmp.h>
#include <wchar.h>
#include <wctype.h>
#include <locale.h>
#include <errno.h>
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

int dummy_SL_IID_BUFFERQUEUE() { return 0; }
int dummy_SL_IID_ENGINE() { return 0; }
int dummy_SL_IID_PLAY() { return 0; }
int dummy_SL_IID_SEEK() { return 0; }
int dummy_SL_IID_VOLUME() { return 0; }
int dummy___FD_SET_chk() { return 0; }
int dummy___cmsg_nxthdr() { return 0; }
int dummy___ctype_get_mb_cur_max() { return 0; }
int dummy___cxa_atexit() { return 0; }
int dummy___cxa_finalize() { return 0; }
int dummy___fread_chk() { return 0; }
int dummy___fwrite_chk() { return 0; }
int dummy___memchr_chk() { return 0; }
int dummy___memcpy_chk() { return 0; }
int dummy___memmove_chk() { return 0; }
int dummy___memset_chk() { return 0; }
int dummy___open_2() { return 0; }
int dummy___poll_chk() { return 0; }
int dummy___read_chk() { return 0; }
int dummy___register_atfork() { return 0; }
int dummy___sF() { return 0; }
int dummy___stack_chk_fail() { return 0; }
int dummy___strchr_chk() { return 0; }
int dummy___strcpy_chk() { return 0; }
int dummy___strlen_chk() { return 0; }
int dummy___strncpy_chk() { return 0; }
int dummy___strncpy_chk2() { return 0; }
int dummy___system_property_get() { return 0; }
int dummy___vsnprintf_chk() { return 0; }
int dummy___vsprintf_chk() { return 0; }
int dummy___write_chk() { return 0; }
int dummy_accept4() { return 0; }
int dummy_android_set_abort_message() { return 0; }
int dummy_deflateReset() { return 0; }
int dummy_dl_iterate_phdr() { return 0; }
int dummy_eventfd() { return 0; }
int dummy_freeifaddrs() { return 0; }
int dummy_freelocale() { return 0; }
int dummy_fseeko() { return 0; }
int dummy_ftello() { return 0; }
int dummy_gai_strerror() { return 0; }
int dummy_getauxval() { return 0; }
int dummy_getentropy() { return 0; }
int dummy_geteuid() { return 0; }
int dummy_getifaddrs() { return 0; }
int dummy_getpwuid_r() { return 0; }
int dummy_gmtime_r() { return 0; }
int dummy_if_nametoindex() { return 0; }
int dummy_inflateReset() { return 0; }
int dummy_inflateReset2() { return 0; }
int dummy_log10f() { return 0; }
int dummy_mbrlen() { return 0; }
int dummy_mbsrtowcs() { return 0; }
int dummy_mbtowc() { return 0; }
int dummy_mlock() { return 0; }
int dummy_newlocale() { return 0; }
int dummy_poll() { return 0; }
int dummy_recvmmsg() { return 0; }
int dummy_select() { return 0; }
int dummy_sendmmsg() { return 0; }
int dummy_shutdown() { return 0; }
int dummy_sigaltstack() { return 0; }
int dummy_signal() { return 0; }
int dummy_sincosf() { return 0; }
int dummy_slCreateEngine() { return 0; }
int dummy_socketpair() { return 0; }
int dummy_strcoll_l() { return 0; }
int dummy_strcspn() { return 0; }
int dummy_strerror_r() { return 0; }
int dummy_strftime_l() { return 0; }
int dummy_strpbrk() { return 0; }
int dummy_strptime() { return 0; }
int dummy_strspn() { return 0; }
int dummy_strtold() { return 0; }
int dummy_strtold_l() { return 0; }
int dummy_strtoll_l() { return 0; }
int dummy_strtoull_l() { return 0; }
int dummy_strxfrm_l() { return 0; }
int dummy_swprintf() { return 0; }
int dummy_syscall() { return 0; }
int dummy_uselocale() { return 0; }
int dummy_vasprintf() { return 0; }
int dummy_vfprintf() { return 0; }
int dummy_wcscoll_l() { return 0; }
int dummy_wcstold() { return 0; }
int dummy_wcsxfrm_l() { return 0; }

DynLibFunction dynlib_functions[] = {
  {"AAssetManager_open", (uintptr_t)&fake_AAssetManager_open},
  {"AAsset_close", (uintptr_t)&fake_AAsset_close},
  {"AAsset_getLength", (uintptr_t)&fake_AAsset_getLength},
  {"AAsset_getRemainingLength", (uintptr_t)&fake_AAsset_getRemainingLength},
  {"AAsset_openFileDescriptor", (uintptr_t)&fake_AAsset_openFileDescriptor},
  {"AAsset_read", (uintptr_t)&fake_AAsset_read},
  {"AAsset_seek", (uintptr_t)&fake_AAsset_seek},
  {"AConfiguration_delete", (uintptr_t)&ret0},
  {"AConfiguration_fromAssetManager", (uintptr_t)&ret0},
  {"AConfiguration_getCountry", (uintptr_t)&ret0},
  {"AConfiguration_getDensity", (uintptr_t)&ret0},
  {"AConfiguration_getLanguage", (uintptr_t)&ret0},
  {"AConfiguration_getScreenLong", (uintptr_t)&ret0},
  {"AConfiguration_getScreenSize", (uintptr_t)&ret0},
  {"AConfiguration_new", (uintptr_t)&ret0},
  {"AInputEvent_getDeviceId", (uintptr_t)&ret0},
  {"AInputEvent_getSource", (uintptr_t)&ret0},
  {"AInputEvent_getType", (uintptr_t)&ret0},
  {"AInputQueue_attachLooper", (uintptr_t)&ret0},
  {"AInputQueue_detachLooper", (uintptr_t)&ret0},
  {"AInputQueue_finishEvent", (uintptr_t)&ret0},
  {"AInputQueue_getEvent", (uintptr_t)&ret0},
  {"AInputQueue_preDispatchEvent", (uintptr_t)&ret0},
  {"AKeyEvent_getAction", (uintptr_t)&ret0},
  {"AKeyEvent_getKeyCode", (uintptr_t)&ret0},
  {"ALooper_addFd", (uintptr_t)&ret0},
  {"ALooper_pollOnce", (uintptr_t)&ret0},
  {"ALooper_prepare", (uintptr_t)&ret0},
  {"AMotionEvent_getAction", (uintptr_t)&ret0},
  {"AMotionEvent_getEventTime", (uintptr_t)&ret0},
  {"AMotionEvent_getPointerCount", (uintptr_t)&ret0},
  {"AMotionEvent_getPointerId", (uintptr_t)&ret0},
  {"AMotionEvent_getX", (uintptr_t)&ret0},
  {"AMotionEvent_getY", (uintptr_t)&ret0},
  {"ANativeActivity_finish", (uintptr_t)&ret0},
  {"ANativeWindow_setBuffersGeometry", (uintptr_t)&ret0},
  {"SL_IID_BUFFERQUEUE", (uintptr_t)&dummy_SL_IID_BUFFERQUEUE},
  {"SL_IID_ENGINE", (uintptr_t)&dummy_SL_IID_ENGINE},
  {"SL_IID_PLAY", (uintptr_t)&dummy_SL_IID_PLAY},
  {"SL_IID_SEEK", (uintptr_t)&dummy_SL_IID_SEEK},
  {"SL_IID_VOLUME", (uintptr_t)&dummy_SL_IID_VOLUME},
  {"__FD_SET_chk", (uintptr_t)&dummy___FD_SET_chk},
  {"__android_log_print", (uintptr_t)&__android_log_print},
  {"__android_log_write", (uintptr_t)&__android_log_write},
  {"__cmsg_nxthdr", (uintptr_t)&dummy___cmsg_nxthdr},
  {"__ctype_get_mb_cur_max", (uintptr_t)&dummy___ctype_get_mb_cur_max},
  {"__cxa_atexit", (uintptr_t)&dummy___cxa_atexit},
  {"__cxa_finalize", (uintptr_t)&dummy___cxa_finalize},
  {"__errno", (uintptr_t)&__errno},
  {"__fread_chk", (uintptr_t)&dummy___fread_chk},
  {"__fwrite_chk", (uintptr_t)&dummy___fwrite_chk},
  {"__memchr_chk", (uintptr_t)&dummy___memchr_chk},
  {"__memcpy_chk", (uintptr_t)&dummy___memcpy_chk},
  {"__memmove_chk", (uintptr_t)&dummy___memmove_chk},
  {"__memset_chk", (uintptr_t)&dummy___memset_chk},
  {"__open_2", (uintptr_t)&dummy___open_2},
  {"__poll_chk", (uintptr_t)&dummy___poll_chk},
  {"__read_chk", (uintptr_t)&dummy___read_chk},
  {"__register_atfork", (uintptr_t)&dummy___register_atfork},
  {"__sF", (uintptr_t)&fake_sF},
  {"__stack_chk_fail", (uintptr_t)&dummy___stack_chk_fail},
  {"__strchr_chk", (uintptr_t)&dummy___strchr_chk},
  {"__strcpy_chk", (uintptr_t)&dummy___strcpy_chk},
  {"__strlen_chk", (uintptr_t)&dummy___strlen_chk},
  {"__strncpy_chk", (uintptr_t)&dummy___strncpy_chk},
  {"__strncpy_chk2", (uintptr_t)&dummy___strncpy_chk2},
  {"__system_property_get", (uintptr_t)&dummy___system_property_get},
  {"__vsnprintf_chk", (uintptr_t)&dummy___vsnprintf_chk},
  {"__vsprintf_chk", (uintptr_t)&dummy___vsprintf_chk},
  {"__write_chk", (uintptr_t)&dummy___write_chk},
  {"abort", (uintptr_t)&abort},
  {"accept", (uintptr_t)&accept},
  {"accept4", (uintptr_t)&dummy_accept4},
  {"access", (uintptr_t)&access},
  {"android_set_abort_message", (uintptr_t)&dummy_android_set_abort_message},
  {"atan", (uintptr_t)&atan},
  {"atan2f", (uintptr_t)&atan2f},
  {"atof", (uintptr_t)&atof},
  {"atoi", (uintptr_t)&atoi},
  {"basename", (uintptr_t)&basename},
  {"bind", (uintptr_t)&bind},
  {"btowc", (uintptr_t)&btowc},
  {"calloc", (uintptr_t)&calloc},
  {"clock_gettime", (uintptr_t)&clock_gettime},
  {"close", (uintptr_t)&close},
  {"closedir", (uintptr_t)&closedir},
  {"closelog", (uintptr_t)&closelog},
  {"connect", (uintptr_t)&connect},
  {"cosf", (uintptr_t)&cosf},
  {"crc32", (uintptr_t)&crc32},
  {"deflate", (uintptr_t)&deflate},
  {"deflateEnd", (uintptr_t)&deflateEnd},
  {"deflateInit2_", (uintptr_t)&deflateInit2_},
  {"deflateReset", (uintptr_t)&dummy_deflateReset},
  {"dl_iterate_phdr", (uintptr_t)&dummy_dl_iterate_phdr},
  {"dlclose", (uintptr_t)&dlclose},
  {"dlerror", (uintptr_t)&dlerror},
  {"dlopen", (uintptr_t)&dlopen},
  {"dlsym", (uintptr_t)&dlsym},
  {"eglChooseConfig", (uintptr_t)&egl_shim_ChooseConfig},
  {"eglCreateContext", (uintptr_t)&egl_shim_CreateContext},
  {"eglCreateWindowSurface", (uintptr_t)&egl_shim_CreateWindowSurface},
  {"eglDestroyContext", (uintptr_t)&egl_shim_DestroyContext},
  {"eglDestroySurface", (uintptr_t)&egl_shim_DestroySurface},
  {"eglGetConfigAttrib", (uintptr_t)&egl_shim_GetConfigAttrib},
  {"eglGetDisplay", (uintptr_t)&egl_shim_GetDisplay},
  {"eglGetError", (uintptr_t)&egl_shim_GetError},
  {"eglGetProcAddress", (uintptr_t)&egl_shim_GetProcAddress},
  {"eglInitialize", (uintptr_t)&egl_shim_Initialize},
  {"eglMakeCurrent", (uintptr_t)&egl_shim_MakeCurrent},
  {"eglQuerySurface", (uintptr_t)&egl_shim_QuerySurface},
  {"eglSwapBuffers", (uintptr_t)&egl_shim_SwapBuffers},
  {"eglTerminate", (uintptr_t)&egl_shim_Terminate},
  {"eventfd", (uintptr_t)&dummy_eventfd},
  {"exit", (uintptr_t)&exit},
  {"fclose", (uintptr_t)&fclose},
  {"fcntl", (uintptr_t)&fcntl},
  {"fdopen", (uintptr_t)&fdopen},
  {"feof", (uintptr_t)&feof},
  {"ferror", (uintptr_t)&ferror},
  {"fflush", (uintptr_t)&fflush},
  {"fgets", (uintptr_t)&fgets},
  {"fileno", (uintptr_t)&fileno},
  {"fopen", (uintptr_t)&fopen},
  {"fprintf", (uintptr_t)&fprintf},
  {"fputc", (uintptr_t)&fputc},
  {"fputs", (uintptr_t)&fputs},
  {"fread", (uintptr_t)&fread},
  {"free", (uintptr_t)&free},
  {"freeaddrinfo", (uintptr_t)&freeaddrinfo},
  {"freeifaddrs", (uintptr_t)&dummy_freeifaddrs},
  {"freelocale", (uintptr_t)&dummy_freelocale},
  {"frexp", (uintptr_t)&frexp},
  {"fseek", (uintptr_t)&fseek},
  {"fseeko", (uintptr_t)&dummy_fseeko},
  {"fstat", (uintptr_t)&fstat},
  {"ftell", (uintptr_t)&ftell},
  {"ftello", (uintptr_t)&dummy_ftello},
  {"fwrite", (uintptr_t)&fwrite},
  {"gai_strerror", (uintptr_t)&dummy_gai_strerror},
  {"getaddrinfo", (uintptr_t)&getaddrinfo},
  {"getauxval", (uintptr_t)&dummy_getauxval},
  {"getentropy", (uintptr_t)&dummy_getentropy},
  {"getenv", (uintptr_t)&getenv},
  {"geteuid", (uintptr_t)&dummy_geteuid},
  {"gethostbyname", (uintptr_t)&gethostbyname},
  {"getifaddrs", (uintptr_t)&dummy_getifaddrs},
  {"getnameinfo", (uintptr_t)&getnameinfo},
  {"getpeername", (uintptr_t)&getpeername},
  {"getpid", (uintptr_t)&getpid},
  {"getpwuid_r", (uintptr_t)&dummy_getpwuid_r},
  {"getsockname", (uintptr_t)&getsockname},
  {"getsockopt", (uintptr_t)&getsockopt},
  {"gettimeofday", (uintptr_t)&gettimeofday},
  {"glActiveTexture", (uintptr_t)&glActiveTexture},
  {"glAttachShader", (uintptr_t)&glAttachShader},
  {"glBindBuffer", (uintptr_t)&glBindBuffer},
  {"glBindFramebuffer", (uintptr_t)&glBindFramebuffer},
  {"glBindRenderbuffer", (uintptr_t)&glBindRenderbuffer},
  {"glBindTexture", (uintptr_t)&glBindTexture},
  {"glBlendFunc", (uintptr_t)&glBlendFunc},
  {"glBufferData", (uintptr_t)&glBufferData},
  {"glCheckFramebufferStatus", (uintptr_t)&glCheckFramebufferStatus},
  {"glClear", (uintptr_t)&glClear},
  {"glClearColor", (uintptr_t)&glClearColor},
  {"glClearDepthf", (uintptr_t)&glClearDepthf},
  {"glColorMask", (uintptr_t)&glColorMask},
  {"glCompileShader", (uintptr_t)&glCompileShader},
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
  {"glDrawArrays", (uintptr_t)&glDrawArrays},
  {"glDrawElements", (uintptr_t)&glDrawElements},
  {"glEnable", (uintptr_t)&glEnable},
  {"glEnableVertexAttribArray", (uintptr_t)&glEnableVertexAttribArray},
  {"glFramebufferRenderbuffer", (uintptr_t)&glFramebufferRenderbuffer},
  {"glFramebufferTexture2D", (uintptr_t)&glFramebufferTexture2D},
  {"glFrontFace", (uintptr_t)&glFrontFace},
  {"glGenBuffers", (uintptr_t)&glGenBuffers},
  {"glGenFramebuffers", (uintptr_t)&glGenFramebuffers},
  {"glGenRenderbuffers", (uintptr_t)&glGenRenderbuffers},
  {"glGenTextures", (uintptr_t)&glGenTextures},
  {"glGetAttribLocation", (uintptr_t)&glGetAttribLocation},
  {"glGetError", (uintptr_t)&glGetError},
  {"glGetIntegerv", (uintptr_t)&glGetIntegerv},
  {"glGetProgramInfoLog", (uintptr_t)&glGetProgramInfoLog},
  {"glGetProgramiv", (uintptr_t)&glGetProgramiv},
  {"glGetShaderInfoLog", (uintptr_t)&glGetShaderInfoLog},
  {"glGetShaderiv", (uintptr_t)&glGetShaderiv},
  {"glGetString", (uintptr_t)&glGetString},
  {"glGetUniformLocation", (uintptr_t)&glGetUniformLocation},
  {"glIsFramebuffer", (uintptr_t)&glIsFramebuffer},
  {"glIsRenderbuffer", (uintptr_t)&glIsRenderbuffer},
  {"glIsTexture", (uintptr_t)&glIsTexture},
  {"glLinkProgram", (uintptr_t)&glLinkProgram},
  {"glRenderbufferStorage", (uintptr_t)&glRenderbufferStorage},
  {"glShaderSource", (uintptr_t)&glShaderSource},
  {"glStencilFunc", (uintptr_t)&glStencilFunc},
  {"glStencilMask", (uintptr_t)&glStencilMask},
  {"glStencilOp", (uintptr_t)&glStencilOp},
  {"glTexImage2D", (uintptr_t)&glTexImage2D},
  {"glTexParameteri", (uintptr_t)&glTexParameteri},
  {"glTexSubImage2D", (uintptr_t)&glTexSubImage2D},
  {"glUniform1i", (uintptr_t)&glUniform1i},
  {"glUniform4f", (uintptr_t)&glUniform4f},
  {"glUseProgram", (uintptr_t)&glUseProgram},
  {"glValidateProgram", (uintptr_t)&glValidateProgram},
  {"glVertexAttribPointer", (uintptr_t)&glVertexAttribPointer},
  {"glViewport", (uintptr_t)&glViewport},
  {"gmtime", (uintptr_t)&gmtime},
  {"gmtime_r", (uintptr_t)&dummy_gmtime_r},
  {"if_nametoindex", (uintptr_t)&dummy_if_nametoindex},
  {"inet_ntop", (uintptr_t)&inet_ntop},
  {"inet_pton", (uintptr_t)&inet_pton},
  {"inflate", (uintptr_t)&inflate},
  {"inflateEnd", (uintptr_t)&inflateEnd},
  {"inflateInit2_", (uintptr_t)&inflateInit2_},
  {"inflateInit_", (uintptr_t)&inflateInit_},
  {"inflateReset", (uintptr_t)&dummy_inflateReset},
  {"inflateReset2", (uintptr_t)&dummy_inflateReset2},
  {"ioctl", (uintptr_t)&ioctl},
  {"iswalpha_l", (uintptr_t)&iswalpha_l},
  {"iswblank_l", (uintptr_t)&iswblank_l},
  {"iswcntrl_l", (uintptr_t)&iswcntrl_l},
  {"iswdigit_l", (uintptr_t)&iswdigit_l},
  {"iswlower_l", (uintptr_t)&iswlower_l},
  {"iswprint_l", (uintptr_t)&iswprint_l},
  {"iswpunct_l", (uintptr_t)&iswpunct_l},
  {"iswspace_l", (uintptr_t)&iswspace_l},
  {"iswupper_l", (uintptr_t)&iswupper_l},
  {"iswxdigit_l", (uintptr_t)&iswxdigit_l},
  {"listen", (uintptr_t)&listen},
  {"localeconv", (uintptr_t)&localeconv},
  {"localtime", (uintptr_t)&localtime},
  {"log10f", (uintptr_t)&dummy_log10f},
  {"longjmp", (uintptr_t)&longjmp},
  {"lseek", (uintptr_t)&lseek},
  {"madvise", (uintptr_t)&madvise},
  {"malloc", (uintptr_t)&malloc},
  {"mbrlen", (uintptr_t)&dummy_mbrlen},
  {"mbrtowc", (uintptr_t)&mbrtowc},
  {"mbsnrtowcs", (uintptr_t)&mbsnrtowcs},
  {"mbsrtowcs", (uintptr_t)&dummy_mbsrtowcs},
  {"mbtowc", (uintptr_t)&dummy_mbtowc},
  {"memchr", (uintptr_t)&memchr},
  {"memcmp", (uintptr_t)&memcmp},
  {"memcpy", (uintptr_t)&memcpy},
  {"memmove", (uintptr_t)&memmove},
  {"memrchr", (uintptr_t)&memrchr},
  {"memset", (uintptr_t)&memset},
  {"mkdir", (uintptr_t)&mkdir},
  {"mktime", (uintptr_t)&mktime},
  {"mlock", (uintptr_t)&dummy_mlock},
  {"mmap", (uintptr_t)&mmap},
  {"modf", (uintptr_t)&modf},
  {"mprotect", (uintptr_t)&mprotect},
  {"munmap", (uintptr_t)&munmap},
  {"nanosleep", (uintptr_t)&nanosleep},
  {"newlocale", (uintptr_t)&dummy_newlocale},
  {"open", (uintptr_t)&open},
  {"opendir", (uintptr_t)&opendir},
  {"openlog", (uintptr_t)&openlog},
  {"pipe", (uintptr_t)&pipe},
  {"poll", (uintptr_t)&dummy_poll},
  {"posix_memalign", (uintptr_t)&posix_memalign},
  {"pow", (uintptr_t)&pow},
  {"printf", (uintptr_t)&printf},
  {"pthread_attr_destroy", (uintptr_t)&pthread_attr_destroy},
  {"pthread_attr_init", (uintptr_t)&pthread_attr_init},
  {"pthread_attr_setdetachstate", (uintptr_t)&pthread_attr_setdetachstate},
  {"pthread_cond_broadcast", (uintptr_t)&pthread_cond_broadcast},
  {"pthread_cond_destroy", (uintptr_t)&pthread_cond_destroy},
  {"pthread_cond_init", (uintptr_t)&pthread_cond_init},
  {"pthread_cond_signal", (uintptr_t)&pthread_cond_signal},
  {"pthread_cond_timedwait", (uintptr_t)&pthread_cond_timedwait},
  {"pthread_cond_wait", (uintptr_t)&pthread_cond_wait},
  {"pthread_create", (uintptr_t)&pthread_create},
  {"pthread_detach", (uintptr_t)&pthread_detach},
  {"pthread_equal", (uintptr_t)&pthread_equal},
  {"pthread_exit", (uintptr_t)&pthread_exit},
  {"pthread_getspecific", (uintptr_t)&pthread_getspecific},
  {"pthread_join", (uintptr_t)&pthread_join},
  {"pthread_key_create", (uintptr_t)&pthread_key_create},
  {"pthread_key_delete", (uintptr_t)&pthread_key_delete},
  {"pthread_mutex_destroy", (uintptr_t)&pthread_mutex_destroy},
  {"pthread_mutex_init", (uintptr_t)&pthread_mutex_init},
  {"pthread_mutex_lock", (uintptr_t)&pthread_mutex_lock},
  {"pthread_mutex_trylock", (uintptr_t)&pthread_mutex_trylock},
  {"pthread_mutex_unlock", (uintptr_t)&pthread_mutex_unlock},
  {"pthread_mutexattr_destroy", (uintptr_t)&pthread_mutexattr_destroy},
  {"pthread_mutexattr_init", (uintptr_t)&pthread_mutexattr_init},
  {"pthread_mutexattr_settype", (uintptr_t)&pthread_mutexattr_settype},
  {"pthread_once", (uintptr_t)&pthread_once},
  {"pthread_rwlock_destroy", (uintptr_t)&pthread_rwlock_destroy},
  {"pthread_rwlock_init", (uintptr_t)&pthread_rwlock_init},
  {"pthread_rwlock_rdlock", (uintptr_t)&pthread_rwlock_rdlock},
  {"pthread_rwlock_unlock", (uintptr_t)&pthread_rwlock_unlock},
  {"pthread_rwlock_wrlock", (uintptr_t)&pthread_rwlock_wrlock},
  {"pthread_self", (uintptr_t)&pthread_self},
  {"pthread_setspecific", (uintptr_t)&pthread_setspecific},
  {"qsort", (uintptr_t)&qsort},
  {"rand", (uintptr_t)&rand},
  {"read", (uintptr_t)&read},
  {"readdir", (uintptr_t)&readdir},
  {"realloc", (uintptr_t)&realloc},
  {"realpath", (uintptr_t)&realpath},
  {"recvfrom", (uintptr_t)&recvfrom},
  {"recvmmsg", (uintptr_t)&dummy_recvmmsg},
  {"remove", (uintptr_t)&remove},
  {"rename", (uintptr_t)&rename},
  {"sched_yield", (uintptr_t)&sched_yield},
  {"select", (uintptr_t)&dummy_select},
  {"sendmmsg", (uintptr_t)&dummy_sendmmsg},
  {"sendto", (uintptr_t)&sendto},
  {"setjmp", (uintptr_t)&setjmp},
  {"setlocale", (uintptr_t)&setlocale},
  {"setsockopt", (uintptr_t)&setsockopt},
  {"setvbuf", (uintptr_t)&setvbuf},
  {"shutdown", (uintptr_t)&dummy_shutdown},
  {"sigaction", (uintptr_t)&sigaction},
  {"sigaltstack", (uintptr_t)&dummy_sigaltstack},
  {"sigemptyset", (uintptr_t)&sigemptyset},
  {"signal", (uintptr_t)&dummy_signal},
  {"sincosf", (uintptr_t)&dummy_sincosf},
  {"sinf", (uintptr_t)&sinf},
  {"slCreateEngine", (uintptr_t)&dummy_slCreateEngine},
  {"snprintf", (uintptr_t)&snprintf},
  {"socket", (uintptr_t)&socket},
  {"socketpair", (uintptr_t)&dummy_socketpair},
  {"srand", (uintptr_t)&srand},
  {"sscanf", (uintptr_t)&sscanf},
  {"stat", (uintptr_t)&stat},
  {"stderr", (uintptr_t)&stderr},
  {"stdin", (uintptr_t)&stdin},
  {"stdout", (uintptr_t)&stdout},
  {"strcat", (uintptr_t)&strcat},
  {"strchr", (uintptr_t)&strchr},
  {"strcmp", (uintptr_t)&strcmp},
  {"strcoll_l", (uintptr_t)&dummy_strcoll_l},
  {"strcpy", (uintptr_t)&strcpy},
  {"strcspn", (uintptr_t)&dummy_strcspn},
  {"strdup", (uintptr_t)&strdup},
  {"strerror", (uintptr_t)&strerror},
  {"strerror_r", (uintptr_t)&dummy_strerror_r},
  {"strftime", (uintptr_t)&strftime},
  {"strftime_l", (uintptr_t)&dummy_strftime_l},
  {"strlen", (uintptr_t)&strlen},
  {"strncmp", (uintptr_t)&strncmp},
  {"strncpy", (uintptr_t)&strncpy},
  {"strpbrk", (uintptr_t)&dummy_strpbrk},
  {"strptime", (uintptr_t)&dummy_strptime},
  {"strrchr", (uintptr_t)&strrchr},
  {"strspn", (uintptr_t)&dummy_strspn},
  {"strstr", (uintptr_t)&strstr},
  {"strtod", (uintptr_t)&strtod},
  {"strtof", (uintptr_t)&strtof},
  {"strtol", (uintptr_t)&strtol},
  {"strtold", (uintptr_t)&dummy_strtold},
  {"strtold_l", (uintptr_t)&dummy_strtold_l},
  {"strtoll", (uintptr_t)&strtoll},
  {"strtoll_l", (uintptr_t)&dummy_strtoll_l},
  {"strtoul", (uintptr_t)&strtoul},
  {"strtoull", (uintptr_t)&strtoull},
  {"strtoull_l", (uintptr_t)&dummy_strtoull_l},
  {"strxfrm_l", (uintptr_t)&dummy_strxfrm_l},
  {"swprintf", (uintptr_t)&dummy_swprintf},
  {"syscall", (uintptr_t)&dummy_syscall},
  {"sysconf", (uintptr_t)&sysconf},
  {"syslog", (uintptr_t)&syslog},
  {"time", (uintptr_t)&time},
  {"tmpnam", (uintptr_t)&tmpnam},
  {"towlower_l", (uintptr_t)&towlower_l},
  {"towupper_l", (uintptr_t)&towupper_l},
  {"unlink", (uintptr_t)&unlink},
  {"uselocale", (uintptr_t)&dummy_uselocale},
  {"usleep", (uintptr_t)&usleep},
  {"vasprintf", (uintptr_t)&dummy_vasprintf},
  {"vfprintf", (uintptr_t)&dummy_vfprintf},
  {"vsnprintf", (uintptr_t)&vsnprintf},
  {"vsscanf", (uintptr_t)&vsscanf},
  {"wcrtomb", (uintptr_t)&wcrtomb},
  {"wcscoll_l", (uintptr_t)&dummy_wcscoll_l},
  {"wcslen", (uintptr_t)&wcslen},
  {"wcsnrtombs", (uintptr_t)&wcsnrtombs},
  {"wcstod", (uintptr_t)&wcstod},
  {"wcstof", (uintptr_t)&wcstof},
  {"wcstol", (uintptr_t)&wcstol},
  {"wcstold", (uintptr_t)&dummy_wcstold},
  {"wcstoll", (uintptr_t)&wcstoll},
  {"wcstoul", (uintptr_t)&wcstoul},
  {"wcstoull", (uintptr_t)&wcstoull},
  {"wcsxfrm_l", (uintptr_t)&dummy_wcsxfrm_l},
  {"wctob", (uintptr_t)&wctob},
  {"wmemchr", (uintptr_t)&wmemchr},
  {"wmemcmp", (uintptr_t)&wmemcmp},
  {"write", (uintptr_t)&write},
};
const int dynlib_functions_count = sizeof(dynlib_functions) / sizeof(dynlib_functions[0]);
