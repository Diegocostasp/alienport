#include "android_bridge.h"
#include <stdlib.h>
#include <string.h>
#include <android/log.h>

// Mock JNI functions
static jint GetVersion(JNIEnv *env) { return JNI_VERSION_1_6; }
static jclass FindClass(JNIEnv *env, const char *name) { return (jclass)0x41414141; }
// ... add more as needed by the game

static struct JNINativeInterface_ jni_native_interface = {
    NULL, NULL, NULL, NULL,
    GetVersion,
    NULL, // DefineClass
    FindClass,
    // ... initialize others with NULL or stubs
};

static struct JNIEnv_ jni_env = { &jni_native_interface };
static struct JNIInvokeInterface_ jni_invoke_interface;
static struct JavaVM_ jni_jvm = { &jni_invoke_interface };

JNIEnv *env_ptr = &jni_env;
JavaVM *jvm_ptr = &jni_jvm;

void jni_init() {
    // Initialize JVM interface if needed
}

// Android Log implementation
void __android_log_print(int prio, const char *tag, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    printf("[%s] ", tag);
    vprintf(fmt, ap);
    printf("\n");
    va_end(ap);
}

DynLibFunction android_imports[] = {
    { "__android_log_print", (uintptr_t)__android_log_print },
    // Add more symbols that the .so imports
};
int num_android_imports = sizeof(android_imports) / sizeof(DynLibFunction);
