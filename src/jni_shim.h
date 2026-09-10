#ifndef JNI_SHIM_H
#define JNI_SHIM_H

#ifdef __cplusplus
extern "C" {
#endif

void *jni_get_env(void);
void *jni_get_vm(void);
void jni_shim_init(const char *gamedir);

#ifdef __cplusplus
}
#endif

#endif /* JNI_SHIM_H */
