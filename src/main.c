/*
 * main.c -- Alien Shooter ARM64 Linux loader
 * EGL created via SDL, game's EGL calls are hooked to use our context.
 */
#include <SDL2/SDL.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

#include "so_util.h"
#include "imports.h"
#include "util.h"
#include "fake_jni.h"

#define SO_NAME "lib/arm64-v8a/libalien_shooter.so"
#define MEMORY_MB 256

/* Globals exported for EGL hooks in imports_extra.c */
SDL_Window *g_sdl_window = NULL;
SDL_GLContext g_sdl_glctx = NULL;
EGLDisplay g_egl_display = EGL_NO_DISPLAY;
EGLSurface g_egl_surface = EGL_NO_SURFACE;
EGLContext g_egl_context = EGL_NO_CONTEXT;
int g_screen_w = 640;
int g_screen_h = 480;

/* ANativeActivity types */
typedef struct ANativeActivity ANativeActivity;
typedef struct ANativeActivityCallbacks {
    void (*onStart)(ANativeActivity *a);
    void (*onResume)(ANativeActivity *a);
    void *(*onSaveInstanceState)(ANativeActivity *a, size_t *sz);
    void (*onPause)(ANativeActivity *a);
    void (*onStop)(ANativeActivity *a);
    void (*onDestroy)(ANativeActivity *a);
    void (*onWindowFocusChanged)(ANativeActivity *a, int f);
    void (*onNativeWindowCreated)(ANativeActivity *a, void *w);
    void (*onNativeWindowResized)(ANativeActivity *a, void *w);
    void (*onNativeWindowRedrawNeeded)(ANativeActivity *a, void *w);
    void (*onNativeWindowDestroyed)(ANativeActivity *a, void *w);
    void (*onInputQueueCreated)(ANativeActivity *a, void *q);
    void (*onInputQueueDestroyed)(ANativeActivity *a, void *q);
    void (*onContentRectChanged)(ANativeActivity *a, const void *r);
    void (*onConfigurationChanged)(ANativeActivity *a);
    void (*onLowMemory)(ANativeActivity *a);
} ANativeActivityCallbacks;

struct ANativeActivity {
    ANativeActivityCallbacks *callbacks;
    void *vm; void *env; void *clazz;
    const char *internalDataPath; const char *externalDataPath;
    int32_t sdkVersion; void *instance; void *assetManager; const char *obbPath;
};

/* Fake JNI */
static jint fake_GetVersion(JNIEnv *e) { return JNI_VERSION_1_6; }
static jclass fake_FindClass(JNIEnv *e, const char *n) { return (jclass)0x41414141; }
static jmethodID fake_GetMethodID(JNIEnv *e, jclass c, const char *n, const char *s) { return (jmethodID)0x42424242; }
static jmethodID fake_GetStaticMethodID(JNIEnv *e, jclass c, const char *n, const char *s) { return (jmethodID)0x43434343; }
static jfieldID fake_GetFieldID(JNIEnv *e, jclass c, const char *n, const char *s) { return (jfieldID)0x44444444; }
static jstring fake_NewStringUTF(JNIEnv *e, const char *s) { return (jstring)(uintptr_t)s; }
static const char *fake_GetStringUTFChars(JNIEnv *e, jstring s, jboolean *c) { if(c) *c=0; return (const char*)(uintptr_t)s; }
static void fake_ReleaseStringUTFChars(JNIEnv *e, jstring s, const char *c) {}
static jobject fake_NewGlobalRef(JNIEnv *e, jobject o) { return o; }
static void fake_DeleteGlobalRef(JNIEnv *e, jobject o) {}
static void fake_DeleteLocalRef(JNIEnv *e, jobject o) {}
static jboolean fake_ExceptionCheck(JNIEnv *e) { return 0; }
static void fake_ExceptionClear(JNIEnv *e) {}
static jint fake_GetJavaVM(JNIEnv *e, JavaVM **v) { return 0; }
static jint fake_AttachCurrentThread(JavaVM *v, JNIEnv **e, void *a) { return 0; }
static jint fake_DetachCurrentThread(JavaVM *v) { return 0; }
static jint fake_GetEnv(JavaVM *v, void **e, jint ver) { return 0; }

static struct JNINativeInterface_ jni_funcs = {0};
static struct JNIInvokeInterface_ jvm_funcs = {0};
static JNIEnv jni_env;
static JavaVM jvm;

static void jni_init(void) {
    memset(&jni_funcs, 0, sizeof(jni_funcs));
    jni_funcs.GetVersion = fake_GetVersion;
    jni_funcs.FindClass = fake_FindClass;
    jni_funcs.GetMethodID = fake_GetMethodID;
    jni_funcs.GetStaticMethodID = fake_GetStaticMethodID;
    jni_funcs.GetFieldID = fake_GetFieldID;
    jni_funcs.NewStringUTF = fake_NewStringUTF;
    jni_funcs.GetStringUTFChars = fake_GetStringUTFChars;
    jni_funcs.ReleaseStringUTFChars = fake_ReleaseStringUTFChars;
    jni_funcs.NewGlobalRef = fake_NewGlobalRef;
    jni_funcs.DeleteGlobalRef = fake_DeleteGlobalRef;
    jni_funcs.DeleteLocalRef = fake_DeleteLocalRef;
    jni_funcs.ExceptionCheck = fake_ExceptionCheck;
    jni_funcs.ExceptionClear = fake_ExceptionClear;
    jni_funcs.GetJavaVM = fake_GetJavaVM;
    jvm_funcs.AttachCurrentThread = fake_AttachCurrentThread;
    jvm_funcs.DetachCurrentThread = fake_DetachCurrentThread;
    jvm_funcs.GetEnv = fake_GetEnv;
    jni_env = &jni_funcs;
    jvm = &jvm_funcs;
}

typedef void (*ANativeActivity_createFunc)(ANativeActivity *, void *, size_t);

#include <signal.h>
static void crash_handler(int sig) {
    const char *name = "UNKNOWN";
    if (sig == SIGSEGV) name = "SIGSEGV";
    else if (sig == SIGBUS) name = "SIGBUS";
    else if (sig == SIGABRT) name = "SIGABRT";
    else if (sig == SIGFPE) name = "SIGFPE";
    fprintf(stderr, "\n*** CRASH: %s (signal %d) ***\n", name, sig);
    fflush(stderr);
    /* Write to log file directly */
    FILE *f = fopen("crash.txt", "w");
    if (f) { fprintf(f, "CRASH: %s (signal %d)\n", name, sig); fclose(f); }
    _exit(1);
}

int main(int argc, char *argv[]) {
    /* Install crash handlers */
    signal(SIGSEGV, crash_handler);
    signal(SIGBUS, crash_handler);
    signal(SIGABRT, crash_handler);
    signal(SIGFPE, crash_handler);

    debugPrintf("Alien Shooter for ARM64 Linux\n");

    struct stat st;
    if (stat(SO_NAME, &st) != 0) {
        debugPrintf("ERROR: Cannot find %s\n", SO_NAME);
        return 1;
    }
    debugPrintf("Found %s (%ld bytes)\n", SO_NAME, st.st_size);

    size_t heap_size = MEMORY_MB * 1024 * 1024;
    void *heap = mmap(NULL, heap_size, PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (heap == MAP_FAILED) { debugPrintf("mmap failed\n"); return 1; }

    if (so_load(SO_NAME, heap, heap_size) < 0) {
        debugPrintf("so_load failed\n"); return 1;
    }
    debugPrintf("so_load() passed.\n");

    so_relocate();
    debugPrintf("so_relocate() passed.\n");

    so_resolve(dynlib_functions, dynlib_numfunctions, 1);
    so_resolve(dynlib_functions_extra, dynlib_numfunctions_extra, 1);
    debugPrintf("so_resolve() passed.\n");

    so_finalize();
    so_flush_caches();
    so_execute_init_array();
    debugPrintf("Init arrays executed.\n");

    jni_init();
    debugPrintf("jni_init() passed.\n");

    /* Create SDL window + GL context */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
        debugPrintf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);

    g_sdl_window = SDL_CreateWindow("Alien Shooter",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!g_sdl_window) {
        debugPrintf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }
    SDL_GetWindowSize(g_sdl_window, &g_screen_w, &g_screen_h);
    debugPrintf("Window: %dx%d, driver: %s\n", g_screen_w, g_screen_h, SDL_GetCurrentVideoDriver());

    g_sdl_glctx = SDL_GL_CreateContext(g_sdl_window);
    if (!g_sdl_glctx) {
        debugPrintf("SDL_GL_CreateContext failed: %s\n", SDL_GetError());
        return 1;
    }

    debugPrintf("GL: %s\n", glGetString(GL_VERSION));

    /* Release context from main thread so game thread can grab it via hook_eglMakeCurrent */
    SDL_GL_MakeCurrent(g_sdl_window, NULL);

    ANativeActivity_createFunc onCreate =
        (ANativeActivity_createFunc)so_find_addr("ANativeActivity_onCreate");
    if (!onCreate) { debugPrintf("onCreate not found!\n"); return 1; }
    debugPrintf("ANativeActivity_onCreate at %p\n", onCreate);

    ANativeActivityCallbacks callbacks;
    memset(&callbacks, 0, sizeof(callbacks));
    ANativeActivity activity;
    memset(&activity, 0, sizeof(activity));
    activity.callbacks = &callbacks;
    activity.vm = &jvm;
    activity.env = &jni_env;
    activity.clazz = (void *)0x42424242;
    activity.internalDataPath = "./data";
    activity.externalDataPath = "./data";
    activity.sdkVersion = 28;
    activity.assetManager = (void *)0x12345678;
    mkdir("data", 0755);

    debugPrintf("Calling ANativeActivity_onCreate...\n");
    onCreate(&activity, NULL, 0);
    debugPrintf("ANativeActivity_onCreate returned.\n");

    debugPrintf("Checking callbacks: start=%p resume=%p winCreated=%p focus=%p\n",
        callbacks.onStart, callbacks.onResume,
        callbacks.onNativeWindowCreated, callbacks.onWindowFocusChanged);

    debugPrintf("Waiting 500ms for game thread...\n");
    usleep(500000);
    debugPrintf("Wait done.\n");

    debugPrintf("Sending lifecycle...\n");
    if (callbacks.onNativeWindowCreated) {
        debugPrintf("  calling onNativeWindowCreated(%p)...\n", g_sdl_window);
        callbacks.onNativeWindowCreated(&activity, g_sdl_window);
        debugPrintf("  onNativeWindowCreated done\n");
    } else { debugPrintf("  onNativeWindowCreated is NULL!\n"); }
    usleep(200000);
    if (callbacks.onStart) { debugPrintf("  calling onStart...\n"); callbacks.onStart(&activity); debugPrintf("  onStart done\n"); }
    if (callbacks.onResume) { debugPrintf("  calling onResume...\n"); callbacks.onResume(&activity); debugPrintf("  onResume done\n"); }
    if (callbacks.onWindowFocusChanged) { debugPrintf("  calling onWindowFocusChanged...\n"); callbacks.onWindowFocusChanged(&activity, 1); debugPrintf("  onWindowFocusChanged done\n"); }

    debugPrintf("Entering main loop...\n");
    SDL_Event event;
    int running = 1;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
        }
        SDL_Delay(16);
    }

    if (callbacks.onPause) callbacks.onPause(&activity);
    if (callbacks.onStop) callbacks.onStop(&activity);
    SDL_GL_DeleteContext(g_sdl_glctx);
    SDL_DestroyWindow(g_sdl_window);
    SDL_Quit();
    return 0;
}
