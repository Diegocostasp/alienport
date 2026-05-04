/*
 * main.c -- Alien Shooter ARM64 Linux loader
 *
 * Loads libalien_shooter.so and simulates ANativeActivity environment.
 * Game manages its own EGL/GL context. We only provide SDL window + lifecycle.
 */

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
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

/* ANativeActivity types */
typedef struct ANativeActivity ANativeActivity;

typedef struct ANativeActivityCallbacks {
    void (*onStart)(ANativeActivity *activity);
    void (*onResume)(ANativeActivity *activity);
    void *(*onSaveInstanceState)(ANativeActivity *activity, size_t *outSize);
    void (*onPause)(ANativeActivity *activity);
    void (*onStop)(ANativeActivity *activity);
    void (*onDestroy)(ANativeActivity *activity);
    void (*onWindowFocusChanged)(ANativeActivity *activity, int hasFocus);
    void (*onNativeWindowCreated)(ANativeActivity *activity, void *window);
    void (*onNativeWindowResized)(ANativeActivity *activity, void *window);
    void (*onNativeWindowRedrawNeeded)(ANativeActivity *activity, void *window);
    void (*onNativeWindowDestroyed)(ANativeActivity *activity, void *window);
    void (*onInputQueueCreated)(ANativeActivity *activity, void *queue);
    void (*onInputQueueDestroyed)(ANativeActivity *activity, void *queue);
    void (*onContentRectChanged)(ANativeActivity *activity, const void *rect);
    void (*onConfigurationChanged)(ANativeActivity *activity);
    void (*onLowMemory)(ANativeActivity *activity);
} ANativeActivityCallbacks;

struct ANativeActivity {
    ANativeActivityCallbacks *callbacks;
    void *vm;
    void *env;
    void *clazz;
    const char *internalDataPath;
    const char *externalDataPath;
    int32_t sdkVersion;
    void *instance;
    void *assetManager;
    const char *obbPath;
};

/* Fake JNI */
static jint fake_GetVersion(JNIEnv *env) { return JNI_VERSION_1_6; }
static jclass fake_FindClass(JNIEnv *env, const char *name) {
    debugPrintf("JNI FindClass: %s\n", name);
    return (jclass)0x41414141;
}
static jmethodID fake_GetMethodID(JNIEnv *env, jclass cls, const char *name, const char *sig) {
    debugPrintf("JNI GetMethodID: %s %s\n", name, sig);
    return (jmethodID)0x42424242;
}
static jmethodID fake_GetStaticMethodID(JNIEnv *env, jclass cls, const char *name, const char *sig) {
    return (jmethodID)0x43434343;
}
static jfieldID fake_GetFieldID(JNIEnv *env, jclass cls, const char *name, const char *sig) {
    return (jfieldID)0x44444444;
}
static jstring fake_NewStringUTF(JNIEnv *env, const char *str) {
    return (jstring)(uintptr_t)str;
}
static const char *fake_GetStringUTFChars(JNIEnv *env, jstring str, jboolean *isCopy) {
    if (isCopy) *isCopy = JNI_FALSE;
    return (const char *)(uintptr_t)str;
}
static void fake_ReleaseStringUTFChars(JNIEnv *env, jstring str, const char *chars) { }
static jobject fake_NewGlobalRef(JNIEnv *env, jobject obj) { return obj; }
static void fake_DeleteGlobalRef(JNIEnv *env, jobject obj) { }
static void fake_DeleteLocalRef(JNIEnv *env, jobject obj) { }
static jboolean fake_ExceptionCheck(JNIEnv *env) { return JNI_FALSE; }
static void fake_ExceptionClear(JNIEnv *env) { }
static jint fake_GetJavaVM(JNIEnv *env, JavaVM **vm) { return JNI_OK; }
static jint fake_AttachCurrentThread(JavaVM *vm, JNIEnv **env, void *args) { return JNI_OK; }
static jint fake_DetachCurrentThread(JavaVM *vm) { return JNI_OK; }
static jint fake_GetEnv(JavaVM *vm, void **env, jint version) { return JNI_OK; }

static struct JNINativeInterface_ jni_functions = {0};
static struct JNIInvokeInterface_ jvm_functions = {0};
static JNIEnv jni_env;
static JavaVM jvm;

static void jni_init(void) {
    memset(&jni_functions, 0, sizeof(jni_functions));
    jni_functions.GetVersion = fake_GetVersion;
    jni_functions.FindClass = fake_FindClass;
    jni_functions.GetMethodID = fake_GetMethodID;
    jni_functions.GetStaticMethodID = fake_GetStaticMethodID;
    jni_functions.GetFieldID = fake_GetFieldID;
    jni_functions.NewStringUTF = fake_NewStringUTF;
    jni_functions.GetStringUTFChars = fake_GetStringUTFChars;
    jni_functions.ReleaseStringUTFChars = fake_ReleaseStringUTFChars;
    jni_functions.NewGlobalRef = fake_NewGlobalRef;
    jni_functions.DeleteGlobalRef = fake_DeleteGlobalRef;
    jni_functions.DeleteLocalRef = fake_DeleteLocalRef;
    jni_functions.ExceptionCheck = fake_ExceptionCheck;
    jni_functions.ExceptionClear = fake_ExceptionClear;
    jni_functions.GetJavaVM = fake_GetJavaVM;

    jvm_functions.AttachCurrentThread = fake_AttachCurrentThread;
    jvm_functions.DetachCurrentThread = fake_DetachCurrentThread;
    jvm_functions.GetEnv = fake_GetEnv;

    jni_env = &jni_functions;
    jvm = &jvm_functions;
}

static SDL_Window *sdl_window = NULL;

typedef void (*ANativeActivity_createFunc)(ANativeActivity *, void *, size_t);

int main(int argc, char *argv[]) {
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
    if (heap == MAP_FAILED) {
        debugPrintf("Failed to allocate %d MB\n", MEMORY_MB);
        return 1;
    }

    debugPrintf("Loading %s...\n", SO_NAME);
    if (so_load(SO_NAME, heap, heap_size) < 0) {
        debugPrintf("Failed to load .so\n");
        return 1;
    }
    debugPrintf("so_load() passed.\n");

    so_relocate();
    debugPrintf("so_relocate() passed.\n");

    so_resolve(dynlib_functions, dynlib_numfunctions, 1);
    so_resolve(dynlib_functions_extra, dynlib_numfunctions_extra, 1);
    debugPrintf("so_resolve() passed.\n");

    so_finalize();
    debugPrintf("so_finalize() passed.\n");

    so_flush_caches();
    so_execute_init_array();
    debugPrintf("Init arrays executed.\n");

    jni_init();
    debugPrintf("jni_init() passed.\n");

    /* SDL only - game creates its own EGL context */
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) < 0) {
        debugPrintf("SDL_Init failed: %s\n", SDL_GetError());
        return 1;
    }

    sdl_window = SDL_CreateWindow("Alien Shooter",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        640, 480, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
    if (!sdl_window) {
        debugPrintf("SDL_CreateWindow failed: %s\n", SDL_GetError());
        return 1;
    }
    debugPrintf("SDL initialized, driver: %s\n", SDL_GetCurrentVideoDriver());

    ANativeActivity_createFunc onCreate =
        (ANativeActivity_createFunc)so_find_addr("ANativeActivity_onCreate");
    if (!onCreate) {
        debugPrintf("ANativeActivity_onCreate not found!\n");
        return 1;
    }
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

    /* Wait for game thread to start its looper */
    usleep(500000);

    debugPrintf("Sending lifecycle events...\n");
    if (callbacks.onNativeWindowCreated) {
        callbacks.onNativeWindowCreated(&activity, sdl_window);
        debugPrintf("  onNativeWindowCreated\n");
    }
    usleep(100000);
    if (callbacks.onStart) {
        callbacks.onStart(&activity);
        debugPrintf("  onStart\n");
    }
    if (callbacks.onResume) {
        callbacks.onResume(&activity);
        debugPrintf("  onResume\n");
    }
    if (callbacks.onWindowFocusChanged) {
        callbacks.onWindowFocusChanged(&activity, 1);
        debugPrintf("  onWindowFocusChanged\n");
    }

    debugPrintf("Entering main loop...\n");
    SDL_Event event;
    int running = 1;
    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;
        }
        SDL_Delay(16);
    }

    if (callbacks.onPause)  callbacks.onPause(&activity);
    if (callbacks.onStop)   callbacks.onStop(&activity);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
    return 0;
}
