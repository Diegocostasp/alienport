#include "jni_shim.h"
#include "asset_shim.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define JNI_VTABLE_SIZE 512

static uintptr_t jni_env_vtable[JNI_VTABLE_SIZE];
static void *jni_env_ptr;

static uintptr_t java_vm_vtable[JNI_VTABLE_SIZE];
static void *java_vm_ptr;

static char g_savedir[512] = "savedata";

/* String storage pool */
#define MAX_JSTRINGS 128
static struct {
  void *handle;
  char value[1024];
} g_jstrings[MAX_JSTRINGS];
static int g_jstring_count = 0;

static void *make_jstring(const char *value) {
  if (!value) value = "";
  int idx = g_jstring_count % MAX_JSTRINGS;
  g_jstring_count++;
  static long long storage[MAX_JSTRINGS];
  g_jstrings[idx].handle = &storage[idx];
  strncpy(g_jstrings[idx].value, value, sizeof(g_jstrings[idx].value) - 1);
  g_jstrings[idx].value[sizeof(g_jstrings[idx].value) - 1] = '\0';
  return g_jstrings[idx].handle;
}

static const char *resolve_jstring(void *jstr) {
  if (!jstr) return "";
  for (int i = 0; i < MAX_JSTRINGS; i++) {
    if (g_jstrings[i].handle == jstr)
      return g_jstrings[i].value;
  }
  return "";
}

/* Tagged IDs */
enum {
  CLASS_UNKNOWN = 0,
  CLASS_COMMON_ACTIVITY,
  CLASS_STRINGS_HELPER,
  CLASS_REGISTRY_ENUMERATOR,
  CLASS_INPUT_DEVICE,
  CLASS_INPUT_DEVICE_HELPER,
  CLASS_NATIVE_LOGGER_FACTORY,
  CLASS_NATIVE_LOGGER_ADAPTER,
  CLASS_CONTEXT,
  CLASS_SHARED_PREFERENCES,
  CLASS_SHARED_PREFERENCES_EDITOR,
  CLASS_WINDOW_MANAGER,
  CLASS_DISPLAY,
  CLASS_DISPLAY_METRICS,
  CLASS_ASSET_PACK_MANAGER,
  CLASS_ASSET_PACK_LOCATION,
  CLASS_TASK,
  CLASS_STRING,
  CLASS_CLASS,
  CLASS_CLASS_LOADER
};

enum {
  MID_UNKNOWN = 0,
  MID_GET_INSTANCE,
  MID_GET_ACTIVITY,
  MID_GET_CONTEXT,
  MID_GET_SHARED_PREFS,
  MID_EDIT,
  MID_PUT_STRING,
  MID_PUT_INT,
  MID_PUT_BOOLEAN,
  MID_APPLY,
  MID_COMMIT,
  MID_GET_STRING,
  MID_GET_INT,
  MID_GET_BOOLEAN,
  MID_GET_DEFAULT_DISPLAY,
  MID_GET_METRICS,
  MID_GET_REAL_METRICS,
  MID_GET_PACKAGE_NAME,
  MID_GET_PACKAGE_MANAGER,
  MID_GET_PACKAGE_INFO,
  MID_GET_ASSETS,
  MID_IS_LEGAL,
  MID_GET_ASSET_PACK_LOCATION,
  MID_ASSETS_PATH,
  MID_ADD_ON_COMPLETE_LISTENER,
  MID_ADD_ON_SUCCESS_LISTENER,
  MID_ADD_ON_FAILURE_LISTENER,
  MID_GET_CLASS_LOADER,
  MID_LOAD_CLASS,
  MID_TO_BYTE_ARRAY
};

enum {
  FID_UNKNOWN = 0,
  FID_WIDTH_PIXELS,
  FID_HEIGHT_PIXELS,
  FID_DENSITY,
  FID_SCALED_DENSITY,
  FID_XDPI,
  FID_YDPI,
  FID_SIGNATURES
};

static uintptr_t stub_class(const char *name) {
  if (!name) return CLASS_UNKNOWN;
  if (strstr(name, "CommonActivity")) return CLASS_COMMON_ACTIVITY;
  if (strstr(name, "StringsHelper")) return CLASS_STRINGS_HELPER;
  if (strstr(name, "RegistryEnumerator")) return CLASS_REGISTRY_ENUMERATOR;
  if (strstr(name, "InputDeviceHelper")) return CLASS_INPUT_DEVICE_HELPER;
  if (strstr(name, "InputDevice")) return CLASS_INPUT_DEVICE;
  if (strstr(name, "NativeLoggerFactory")) return CLASS_NATIVE_LOGGER_FACTORY;
  if (strstr(name, "NativeLoggerAdapter")) return CLASS_NATIVE_LOGGER_ADAPTER;
  if (strstr(name, "SharedPreferences$Editor")) return CLASS_SHARED_PREFERENCES_EDITOR;
  if (strstr(name, "SharedPreferences")) return CLASS_SHARED_PREFERENCES;
  if (strstr(name, "Context")) return CLASS_CONTEXT;
  if (strstr(name, "WindowManager")) return CLASS_WINDOW_MANAGER;
  if (strstr(name, "DisplayMetrics")) return CLASS_DISPLAY_METRICS;
  if (strstr(name, "Display")) return CLASS_DISPLAY;
  if (strstr(name, "AssetPackManager")) return CLASS_ASSET_PACK_MANAGER;
  if (strstr(name, "AssetPackLocation")) return CLASS_ASSET_PACK_LOCATION;
  if (strstr(name, "Task")) return CLASS_TASK;
  if (strstr(name, "ClassLoader")) return CLASS_CLASS_LOADER;
  if (strstr(name, "Class")) return CLASS_CLASS;
  if (strstr(name, "String")) return CLASS_STRING;

  // printf("[jni] FindClass: %s (tagged generic)\n", name);
  return CLASS_COMMON_ACTIVITY;
}

static uintptr_t stub_method(const char *name, const char *sig) {
  (void)sig;
  if (!name) return MID_UNKNOWN;
  if (!strcmp(name, "getInstance") || !strcmp(name, "getActivity")) return MID_GET_INSTANCE;
  if (!strcmp(name, "getContext")) return MID_GET_CONTEXT;
  if (!strcmp(name, "getSharedPreferences")) return MID_GET_SHARED_PREFS;
  if (!strcmp(name, "edit")) return MID_EDIT;
  if (!strcmp(name, "putString")) return MID_PUT_STRING;
  if (!strcmp(name, "putInt")) return MID_PUT_INT;
  if (!strcmp(name, "putBoolean")) return MID_PUT_BOOLEAN;
  if (!strcmp(name, "apply")) return MID_APPLY;
  if (!strcmp(name, "commit")) return MID_COMMIT;
  if (!strcmp(name, "getString")) return MID_GET_STRING;
  if (!strcmp(name, "getInt")) return MID_GET_INT;
  if (!strcmp(name, "getBoolean")) return MID_GET_BOOLEAN;
  if (!strcmp(name, "getDefaultDisplay")) return MID_GET_DEFAULT_DISPLAY;
  if (!strcmp(name, "getMetrics") || !strcmp(name, "getRealMetrics")) return MID_GET_METRICS;
  if (!strcmp(name, "getPackageName")) return MID_GET_PACKAGE_NAME;
  if (!strcmp(name, "getPackageManager")) return MID_GET_PACKAGE_MANAGER;
  if (!strcmp(name, "getPackageInfo")) return MID_GET_PACKAGE_INFO;
  if (!strcmp(name, "isLegal")) return MID_IS_LEGAL;
  if (!strcmp(name, "getAssetPackLocation")) return MID_GET_ASSET_PACK_LOCATION;
  if (!strcmp(name, "assetsPath")) return MID_ASSETS_PATH;
  if (!strcmp(name, "addOnCompleteListener")) return MID_ADD_ON_COMPLETE_LISTENER;
  if (!strcmp(name, "addOnSuccessListener")) return MID_ADD_ON_SUCCESS_LISTENER;
  if (!strcmp(name, "addOnFailureListener")) return MID_ADD_ON_FAILURE_LISTENER;
  if (!strcmp(name, "getClassLoader")) return MID_GET_CLASS_LOADER;
  if (!strcmp(name, "loadClass")) return MID_LOAD_CLASS;
  if (!strcmp(name, "toByteArray")) return MID_TO_BYTE_ARRAY;

  // printf("[jni] GetMethodID: %s %s\n", name, sig ? sig : "");
  return MID_GET_INSTANCE;
}

static uintptr_t stub_field(const char *name, const char *sig) {
  (void)sig;
  if (!name) return FID_UNKNOWN;
  if (!strcmp(name, "widthPixels")) return FID_WIDTH_PIXELS;
  if (!strcmp(name, "heightPixels")) return FID_HEIGHT_PIXELS;
  if (!strcmp(name, "density")) return FID_DENSITY;
  if (!strcmp(name, "scaledDensity")) return FID_SCALED_DENSITY;
  if (!strcmp(name, "xdpi")) return FID_XDPI;
  if (!strcmp(name, "ydpi")) return FID_YDPI;
  if (!strcmp(name, "signatures")) return FID_SIGNATURES;
  return FID_WIDTH_PIXELS;
}

/* JNI Functions */
static void *jni_FindClass(void *env, const char *name) {
  (void)env;
  uintptr_t tag = stub_class(name);
  return (void *)tag;
}

static void *jni_GetMethodID(void *env, void *clazz, const char *name, const char *sig) {
  (void)env; (void)clazz;
  return (void *)stub_method(name, sig);
}

static void *jni_GetStaticMethodID(void *env, void *clazz, const char *name, const char *sig) {
  (void)env; (void)clazz;
  return (void *)stub_method(name, sig);
}

static void *jni_GetFieldID(void *env, void *clazz, const char *name, const char *sig) {
  (void)env; (void)clazz;
  return (void *)stub_field(name, sig);
}

static void *jni_GetStaticFieldID(void *env, void *clazz, const char *name, const char *sig) {
  (void)env; (void)clazz;
  return (void *)stub_field(name, sig);
}

static void *jni_CallObjectMethod(void *env, void *obj, void *methodID, ...) {
  (void)env; (void)obj;
  uintptr_t mid = (uintptr_t)methodID;
  if (mid == MID_GET_PACKAGE_NAME) return make_jstring("com.sigmateam.alienshootermobile");
  if (mid == MID_GET_STRING) return make_jstring("");
  if (mid == MID_ASSETS_PATH) return make_jstring(asset_shim_get_gamedir());
  return (void *)1;
}

static void *jni_CallStaticObjectMethod(void *env, void *clazz, void *methodID, ...) {
  (void)env; (void)clazz;
  uintptr_t mid = (uintptr_t)methodID;
  if (mid == MID_GET_PACKAGE_NAME) return make_jstring("com.sigmateam.alienshootermobile");
  return (void *)1;
}

static int jni_CallIntMethod(void *env, void *obj, void *methodID, ...) {
  (void)env; (void)obj;
  uintptr_t mid = (uintptr_t)methodID;
  if (mid == MID_GET_INT) return 0;
  return 1;
}

static int jni_CallStaticIntMethod(void *env, void *clazz, void *methodID, ...) {
  (void)env; (void)clazz; (void)methodID;
  return 1;
}

static unsigned char jni_CallBooleanMethod(void *env, void *obj, void *methodID, ...) {
  (void)env; (void)obj;
  uintptr_t mid = (uintptr_t)methodID;
  if (mid == MID_IS_LEGAL) return 1;
  return 1;
}

static unsigned char jni_CallStaticBooleanMethod(void *env, void *clazz, void *methodID, ...) {
  (void)env; (void)clazz; (void)methodID;
  return 1;
}

static float jni_CallFloatMethod(void *env, void *obj, void *methodID, ...) {
  (void)env; (void)obj; (void)methodID;
  return 0.0f;
}

static void jni_CallVoidMethod(void *env, void *obj, void *methodID, ...) {
  (void)env; (void)obj; (void)methodID;
}

static void jni_CallStaticVoidMethod(void *env, void *clazz, void *methodID, ...) {
  (void)env; (void)clazz; (void)methodID;
}

static int jni_GetIntField(void *env, void *obj, void *fieldID) {
  (void)env; (void)obj;
  uintptr_t fid = (uintptr_t)fieldID;
  if (fid == FID_WIDTH_PIXELS) return 640;
  if (fid == FID_HEIGHT_PIXELS) return 480;
  return 0;
}

static float jni_GetFloatField(void *env, void *obj, void *fieldID) {
  (void)env; (void)obj;
  uintptr_t fid = (uintptr_t)fieldID;
  if (fid == FID_DENSITY || fid == FID_SCALED_DENSITY) return 1.0f;
  if (fid == FID_XDPI || fid == FID_YDPI) return 160.0f;
  return 1.0f;
}

static void *jni_GetObjectField(void *env, void *obj, void *fieldID) {
  (void)env; (void)obj; (void)fieldID;
  return (void *)1;
}

static const char *jni_GetStringUTFChars(void *env, void *string, unsigned char *isCopy) {
  (void)env;
  if (isCopy) *isCopy = 0;
  return resolve_jstring(string);
}

static void jni_ReleaseStringUTFChars(void *env, void *string, const char *utf) {
  (void)env; (void)string; (void)utf;
}

static void *jni_NewStringUTF(void *env, const char *bytes) {
  (void)env;
  return make_jstring(bytes);
}

static int jni_RegisterNatives(void *env, void *clazz, const void *methods, int nMethods) {
  (void)env; (void)clazz; (void)methods; (void)nMethods;
  printf("[jni] RegisterNatives called for %d methods\n", nMethods);
  return 0;
}

static int jni_GetJavaVM(void *env, void **vm) {
  (void)env;
  if (vm) *vm = java_vm_ptr;
  return 0;
}

static void *jni_NewGlobalRef(void *env, void *obj) {
  (void)env;
  return obj;
}

static void jni_DeleteGlobalRef(void *env, void *obj) {
  (void)env; (void)obj;
}

static void jni_DeleteLocalRef(void *env, void *obj) {
  (void)env; (void)obj;
}

static int jni_PushLocalFrame(void *env, int capacity) {
  (void)env; (void)capacity;
  return 0;
}

static void *jni_PopLocalFrame(void *env, void *result) {
  (void)env;
  return result;
}

/* JavaVM implementation */
static int vm_GetEnv(void *vm, void **env, int version) {
  (void)vm; (void)version;
  if (env) *env = jni_env_ptr;
  return 0;
}

static int vm_AttachCurrentThread(void *vm, void **p_env, void *thr_args) {
  (void)vm; (void)thr_args;
  if (p_env) *p_env = jni_env_ptr;
  return 0;
}

static int vm_DetachCurrentThread(void *vm) {
  (void)vm;
  return 0;
}

static int jni_stub_ret0(void) { return 0; }

void jni_shim_init(const char *gamedir) {
  if (gamedir) {
    snprintf(g_savedir, sizeof(g_savedir), "%s/savedata", gamedir);
  }

  // Pre-fill vtables with safe no-op functions
  for (int i = 0; i < JNI_VTABLE_SIZE; i++) {
    jni_env_vtable[i] = (uintptr_t)jni_stub_ret0;
    java_vm_vtable[i] = (uintptr_t)jni_stub_ret0;
  }

  /* JavaVM vtable offsets */
  java_vm_vtable[3] = (uintptr_t)vm_DetachCurrentThread;
  java_vm_vtable[4] = (uintptr_t)vm_AttachCurrentThread;
  java_vm_vtable[6] = (uintptr_t)vm_GetEnv;
  java_vm_vtable[7] = (uintptr_t)vm_AttachCurrentThread;

  /* JNIEnv vtable standard indices */
  jni_env_vtable[6] = (uintptr_t)jni_FindClass;
  jni_env_vtable[19] = (uintptr_t)jni_PushLocalFrame;
  jni_env_vtable[20] = (uintptr_t)jni_PopLocalFrame;
  jni_env_vtable[21] = (uintptr_t)jni_NewGlobalRef;
  jni_env_vtable[22] = (uintptr_t)jni_DeleteGlobalRef;
  jni_env_vtable[23] = (uintptr_t)jni_DeleteLocalRef;
  jni_env_vtable[31] = (uintptr_t)jni_GetMethodID;
  jni_env_vtable[34] = (uintptr_t)jni_CallObjectMethod;
  jni_env_vtable[49] = (uintptr_t)jni_CallBooleanMethod;
  jni_env_vtable[55] = (uintptr_t)jni_CallIntMethod;
  jni_env_vtable[61] = (uintptr_t)jni_CallFloatMethod;
  jni_env_vtable[67] = (uintptr_t)jni_CallVoidMethod;
  jni_env_vtable[94] = (uintptr_t)jni_GetFieldID;
  jni_env_vtable[95] = (uintptr_t)jni_GetObjectField;
  jni_env_vtable[100] = (uintptr_t)jni_GetIntField;
  jni_env_vtable[102] = (uintptr_t)jni_GetFloatField;
  jni_env_vtable[113] = (uintptr_t)jni_GetStaticMethodID;
  jni_env_vtable[116] = (uintptr_t)jni_CallStaticObjectMethod;
  jni_env_vtable[131] = (uintptr_t)jni_CallStaticBooleanMethod;
  jni_env_vtable[137] = (uintptr_t)jni_CallStaticIntMethod;
  jni_env_vtable[149] = (uintptr_t)jni_CallStaticVoidMethod;
  jni_env_vtable[144] = (uintptr_t)jni_GetStaticFieldID;
  jni_env_vtable[163] = (uintptr_t)jni_NewStringUTF;
  jni_env_vtable[165] = (uintptr_t)jni_GetStringUTFChars;
  jni_env_vtable[166] = (uintptr_t)jni_ReleaseStringUTFChars;
  jni_env_vtable[215] = (uintptr_t)jni_RegisterNatives;
  jni_env_vtable[219] = (uintptr_t)jni_GetJavaVM;

  static uintptr_t env_slot;
  env_slot = (uintptr_t)jni_env_vtable;
  jni_env_ptr = &env_slot;

  static uintptr_t vm_slot;
  vm_slot = (uintptr_t)java_vm_vtable;
  java_vm_ptr = &vm_slot;
}

void *jni_get_env(void) { return jni_env_ptr; }
void *jni_get_vm(void) { return java_vm_ptr; }
