/*
 * main.c -- Alien Shooter ARM64 Linux loader
 * EGL created via SDL, game's EGL calls are hooked to use our context.
 */
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <SDL2/SDL.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

#include "fake_jni.h"
#include "imports.h"
#include "so_util.h"
#include "util.h"

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
static jint fake_GetVersion(JNIEnv *e) { return JNI_VERSION_1_6; }
static jclass fake_FindClass(JNIEnv *e, const char *n) {
  debugPrintf("FindClass: %s\n", n);
  return (jclass)0x41414141;
}
static jmethodID fake_GetMethodID(JNIEnv *e, jclass c, const char *n,
                                  const char *s) {
  debugPrintf("GetMethodID: %s %s\n", n, s);
  return (jmethodID)0x42424242;
}
static jmethodID fake_GetStaticMethodID(JNIEnv *e, jclass c, const char *n,
                                        const char *s) {
  debugPrintf("GetStaticMethodID: %s %s\n", n, s);
  return (jmethodID)0x43434343;
}
static jfieldID fake_GetFieldID(JNIEnv *e, jclass c, const char *n,
                                const char *s) {
  debugPrintf("GetFieldID: %s %s\n", n, s);
  return (jfieldID)0x44444444;
}
static jstring fake_NewStringUTF(JNIEnv *e, const char *s) {
  return (jstring)(uintptr_t)s;
}
static const char *fake_GetStringUTFChars(JNIEnv *e, jstring s, jboolean *c) {
  if (c)
    *c = 0;
  return (const char *)(uintptr_t)s;
}
static void fake_ReleaseStringUTFChars(JNIEnv *e, jstring s, const char *c) {}
static jobject fake_NewGlobalRef(JNIEnv *e, jobject o) { return o; }
static void fake_DeleteGlobalRef(JNIEnv *e, jobject o) {}
static void fake_DeleteLocalRef(JNIEnv *e, jobject o) {}
static jboolean fake_ExceptionCheck(JNIEnv *e) { return 0; }
static void fake_ExceptionClear(JNIEnv *e) {}
static jint fake_GetJavaVM(JNIEnv *e, JavaVM **v) { return 0; }
static struct JNINativeInterface_ jni_funcs;
static JNIEnv jni_env;

static jint fake_AttachCurrentThread(JavaVM *vm, JNIEnv **env, void *args) {
  if (env)
    *env = &jni_env;
  return JNI_OK;
}
static jint fake_DetachCurrentThread(JavaVM *v) { return JNI_OK; }
static jint fake_GetEnv(JavaVM *vm, void **env, jint ver) {
  if (env)
    *env = &jni_env;
  return JNI_OK;
}

static struct JNIInvokeInterface_ jvm_funcs = {0};
static JavaVM jvm;

#include "jni_stubs.h"

static jint fake_PushLocalFrame(JNIEnv *e, jint cap) { return 0; }
static jobject fake_PopLocalFrame(JNIEnv *e, jobject res) { return res; }
static jthrowable fake_ExceptionOccurred(JNIEnv *e) { return NULL; }
static jclass fake_GetObjectClass(JNIEnv *e, jobject o) {
  return (jclass)0x45454545;
}
static char dummy_string[256] = "./data";
static jobject fake_CallObjectMethodV(JNIEnv *e, jobject o, jmethodID m,
                                      va_list args) {
  debugPrintf("CallObjectMethodV called\n");
  return (jobject)dummy_string;
}
static jboolean fake_CallBooleanMethodV(JNIEnv *e, jobject o, jmethodID m,
                                        va_list args) {
  return 0;
}
static jint fake_CallIntMethodV(JNIEnv *e, jobject o, jmethodID m,
                                va_list args) {
  return 100;
}

static jint fake_CallStaticIntMethodV(JNIEnv *e, jclass c, jmethodID m, va_list args) { 
    debugPrintf("CallStaticIntMethodV called\n");
    return 28; 
}
static jobject fake_CallStaticObjectMethodV(JNIEnv *e, jclass c, jmethodID m, va_list args) { 
    debugPrintf("CallStaticObjectMethodV called\n");
    return (jobject)dummy_string; 
}
static jsize fake_GetArrayLength(JNIEnv *e, jarray array) { return 1; }
static jobject fake_GetObjectArrayElement(JNIEnv *e, jobjectArray a, jsize i) { return (jobject)dummy_string; }

static void jni_init(void) {
  /* Fill entire struct with the generic stub to prevent NULL pointer calls */
  void **func_array = (void **)&jni_funcs;
  size_t num_funcs = sizeof(jni_funcs) / sizeof(void *);
  for (size_t i = 0; i < num_funcs; i++) {
    func_array[i] = fake_jni_stub_array[i];
  }

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

  jni_funcs.PushLocalFrame = fake_PushLocalFrame;
  jni_funcs.PopLocalFrame = fake_PopLocalFrame;
  jni_funcs.ExceptionOccurred = fake_ExceptionOccurred;
  jni_funcs.GetObjectClass = fake_GetObjectClass;
  jni_funcs.CallObjectMethodV = fake_CallObjectMethodV;
  jni_funcs.CallBooleanMethodV = fake_CallBooleanMethodV;
  jni_funcs.CallIntMethodV = fake_CallIntMethodV;
  jni_funcs.CallStaticIntMethodV = fake_CallStaticIntMethodV;
  jni_funcs.CallStaticObjectMethodV = fake_CallStaticObjectMethodV;
  jni_funcs.GetArrayLength = fake_GetArrayLength;
  jni_funcs.GetObjectArrayElement = fake_GetObjectArrayElement;

  jvm_funcs.AttachCurrentThread = fake_AttachCurrentThread;
  jvm_funcs.DetachCurrentThread = fake_DetachCurrentThread;
  jvm_funcs.GetEnv = fake_GetEnv;

  jni_env = &jni_funcs;
  jvm = &jvm_funcs;
}

typedef void (*ANativeActivity_createFunc)(ANativeActivity *, void *, size_t);

#include <execinfo.h>
#include <signal.h>
#include <ucontext.h>

extern void *text_base;
extern size_t text_size;
extern size_t data_size;

static void crash_handler(int sig, siginfo_t *info, void *ucontext) {
  const char *name = "UNKNOWN";
  if (sig == SIGSEGV)
    name = "SIGSEGV";
  else if (sig == SIGBUS)
    name = "SIGBUS";
  else if (sig == SIGABRT)
    name = "SIGABRT";
  else if (sig == SIGFPE)
    name = "SIGFPE";

  ucontext_t *uc = (ucontext_t *)ucontext;
  uintptr_t pc = uc ? uc->uc_mcontext.pc : 0;
  uintptr_t so_start = (uintptr_t)text_base;
  uintptr_t so_end = so_start + text_size + data_size;

  fprintf(stderr, "\n*** CRASH: %s (signal %d) ***\n", name, sig);
  fprintf(stderr, "  Fault addr: %p\n", info ? info->si_addr : NULL);
  fprintf(stderr, "  PC:         %p\n", (void *)pc);
  if (pc >= so_start && pc < so_end)
    fprintf(stderr, "  SO offset:  0x%lx\n", pc - so_start);
  else
    fprintf(stderr, "  PC is OUTSIDE .so (loader/libc code)\n");

  if (uc) {
    uintptr_t lr = uc->uc_mcontext.regs[30];
    if (lr >= so_start && lr < so_end)
      fprintf(stderr, "  LR offset:  0x%lx\n", lr - so_start);

    fprintf(stderr, "  Registers:\n");
    fprintf(stderr,
            "  x0 : %016llx  x1 : %016llx  x2 : %016llx  x3 : %016llx\n",
            uc->uc_mcontext.regs[0], uc->uc_mcontext.regs[1],
            uc->uc_mcontext.regs[2], uc->uc_mcontext.regs[3]);
    fprintf(stderr,
            "  x4 : %016llx  x5 : %016llx  x6 : %016llx  x7 : %016llx\n",
            uc->uc_mcontext.regs[4], uc->uc_mcontext.regs[5],
            uc->uc_mcontext.regs[6], uc->uc_mcontext.regs[7]);
    fprintf(stderr,
            "  x8 : %016llx  x29: %016llx  x30: %016llx  sp : %016llx\n",
            uc->uc_mcontext.regs[8], uc->uc_mcontext.regs[29],
            uc->uc_mcontext.regs[30], uc->uc_mcontext.sp);
  }

  void *bt[32];
  int bt_size = backtrace(bt, 32);
  fprintf(stderr, "\n  Backtrace:\n");
  backtrace_symbols_fd(bt, bt_size, fileno(stderr));

  fflush(stderr);

  FILE *f = fopen("crash.txt", "w");
  if (f) {
    fprintf(f, "CRASH: %s (signal %d)\nFault: %p\nPC: %p\n", name, sig,
            info ? info->si_addr : NULL, (void *)pc);
    if (pc >= so_start && pc < so_end)
      fprintf(f, "SO offset: 0x%lx\n", pc - so_start);
    fclose(f);
  }
  _exit(1);
}

int main(int argc, char *argv[]) {
  /* Install crash handlers with SA_SIGINFO for detailed info */
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_sigaction = crash_handler;
  sa.sa_flags = SA_SIGINFO;
  sigaction(SIGSEGV, &sa, NULL);
  sigaction(SIGBUS, &sa, NULL);
  sigaction(SIGABRT, &sa, NULL);
  sigaction(SIGFPE, &sa, NULL);

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
    debugPrintf("mmap failed\n");
    return 1;
  }

  if (so_load(SO_NAME, heap, heap_size) < 0) {
    debugPrintf("so_load failed\n");
    return 1;
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

  /* Disable SDL's signal handlers so our crash handler gets the real PC and
   * fault addr */
  SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");

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

  g_sdl_window = SDL_CreateWindow(
      "Alien Shooter", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480,
      SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);
  if (!g_sdl_window) {
    debugPrintf("SDL_CreateWindow failed: %s\n", SDL_GetError());
    return 1;
  }
  SDL_GetWindowSize(g_sdl_window, &g_screen_w, &g_screen_h);
  debugPrintf("Window: %dx%d, driver: %s\n", g_screen_w, g_screen_h,
              SDL_GetCurrentVideoDriver());

  g_sdl_glctx = SDL_GL_CreateContext(g_sdl_window);
  if (!g_sdl_glctx) {
    debugPrintf("SDL_GL_CreateContext failed: %s\n", SDL_GetError());
    return 1;
  }

  debugPrintf("GL: %s\n", glGetString(GL_VERSION));

  /* Release context from main thread so game thread can grab it via
   * hook_eglMakeCurrent */
  SDL_GL_MakeCurrent(g_sdl_window, NULL);

  ANativeActivity_createFunc onCreate =
      (ANativeActivity_createFunc)so_find_addr("ANativeActivity_onCreate");
  if (!onCreate) {
    debugPrintf("onCreate not found!\n");
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
  } else {
    debugPrintf("  onNativeWindowCreated is NULL!\n");
  }
  usleep(200000);
  if (callbacks.onStart) {
    debugPrintf("  calling onStart...\n");
    callbacks.onStart(&activity);
    debugPrintf("  onStart done\n");
  }
  if (callbacks.onResume) {
    debugPrintf("  calling onResume...\n");
    callbacks.onResume(&activity);
    debugPrintf("  onResume done\n");
  }
  if (callbacks.onWindowFocusChanged) {
    debugPrintf("  calling onWindowFocusChanged...\n");
    callbacks.onWindowFocusChanged(&activity, 1);
    debugPrintf("  onWindowFocusChanged done\n");
  }

  debugPrintf("Entering main loop...\n");
  SDL_Event event;
  int running = 1;
  while (running) {
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT)
        running = 0;
    }
    SDL_Delay(16);
  }

  if (callbacks.onPause)
    callbacks.onPause(&activity);
  if (callbacks.onStop)
    callbacks.onStop(&activity);
  SDL_GL_DeleteContext(g_sdl_glctx);
  SDL_DestroyWindow(g_sdl_window);
  SDL_Quit();
  return 0;
}
