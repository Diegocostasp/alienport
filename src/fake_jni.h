/*
 * fake_jni.h -- Minimal JNI type definitions for running Android .so on Linux
 *
 * We only define the types and structs that ANativeActivity_onCreate needs.
 * No actual JNI implementation — just enough to compile and link.
 */

#ifndef FAKE_JNI_H
#define FAKE_JNI_H

#include <stdint.h>
#include <stddef.h>

/* Basic JNI types */
typedef uint8_t  jboolean;
typedef int8_t   jbyte;
typedef uint16_t jchar;
typedef int16_t  jshort;
typedef int32_t  jint;
typedef int64_t  jlong;
typedef float    jfloat;
typedef double   jdouble;
typedef jint     jsize;

#define JNI_FALSE 0
#define JNI_TRUE  1
#define JNI_OK    0
#define JNI_ERR   (-1)

#define JNI_VERSION_1_1 0x00010001
#define JNI_VERSION_1_2 0x00010002
#define JNI_VERSION_1_4 0x00010004
#define JNI_VERSION_1_6 0x00010006

/* Reference types — all just opaque pointers */
typedef void* jobject;
typedef jobject jclass;
typedef jobject jstring;
typedef jobject jarray;
typedef jobject jobjectArray;
typedef jobject jbooleanArray;
typedef jobject jbyteArray;
typedef jobject jcharArray;
typedef jobject jshortArray;
typedef jobject jintArray;
typedef jobject jlongArray;
typedef jobject jfloatArray;
typedef jobject jdoubleArray;
typedef jobject jthrowable;
typedef jobject jweak;

typedef struct _jfieldID* jfieldID;
typedef struct _jmethodID* jmethodID;

/* JNI Native Method structure */
typedef struct {
    const char *name;
    const char *signature;
    void       *fnPtr;
} JNINativeMethod;

/* Forward declarations */
struct JNINativeInterface_;
struct JNIInvokeInterface_;

typedef const struct JNINativeInterface_* JNIEnv;
typedef const struct JNIInvokeInterface_* JavaVM;

/*
 * JNINativeInterface_ — the massive function table that JNIEnv points to.
 * We define just enough slots. The game accesses these by index,
 * so the order and count matter. Full JNI has 234 entries.
 * We fill all with NULL and only implement the ones we need.
 */
struct JNINativeInterface_ {
    void *reserved0;
    void *reserved1;
    void *reserved2;
    void *reserved3;

    jint        (*GetVersion)(JNIEnv*);                                          /* 4 */
    jclass      (*DefineClass)(JNIEnv*, const char*, jobject, const jbyte*, jsize); /* 5 */
    jclass      (*FindClass)(JNIEnv*, const char*);                              /* 6 */
    jmethodID   (*FromReflectedMethod)(JNIEnv*, jobject);                        /* 7 */
    jfieldID    (*FromReflectedField)(JNIEnv*, jobject);                         /* 8 */
    jobject     (*ToReflectedMethod)(JNIEnv*, jclass, jmethodID, jboolean);      /* 9 */
    jclass      (*GetSuperclass)(JNIEnv*, jclass);                               /* 10 */
    jboolean    (*IsAssignableFrom)(JNIEnv*, jclass, jclass);                    /* 11 */
    jobject     (*ToReflectedField)(JNIEnv*, jclass, jfieldID, jboolean);        /* 12 */
    jint        (*Throw)(JNIEnv*, jthrowable);                                   /* 13 */
    jint        (*ThrowNew)(JNIEnv*, jclass, const char*);                       /* 14 */
    jthrowable  (*ExceptionOccurred)(JNIEnv*);                                   /* 15 */
    void        (*ExceptionDescribe)(JNIEnv*);                                   /* 16 */
    void        (*ExceptionClear)(JNIEnv*);                                      /* 17 */
    void        (*FatalError)(JNIEnv*, const char*);                             /* 18 */
    jint        (*PushLocalFrame)(JNIEnv*, jint);                                /* 19 */
    jobject     (*PopLocalFrame)(JNIEnv*, jobject);                               /* 20 */
    jobject     (*NewGlobalRef)(JNIEnv*, jobject);                               /* 21 */
    void        (*DeleteGlobalRef)(JNIEnv*, jobject);                            /* 22 */
    void        (*DeleteLocalRef)(JNIEnv*, jobject);                             /* 23 */
    jboolean    (*IsSameObject)(JNIEnv*, jobject, jobject);                      /* 24 */
    jobject     (*NewLocalRef)(JNIEnv*, jobject);                                /* 25 */
    jint        (*EnsureLocalCapacity)(JNIEnv*, jint);                           /* 26 */
    jobject     (*AllocObject)(JNIEnv*, jclass);                                 /* 27 */
    jobject     (*NewObject)(JNIEnv*, jclass, jmethodID, ...);                   /* 28 */
    jobject     (*NewObjectV)(JNIEnv*, jclass, jmethodID, void*);                /* 29 */
    jobject     (*NewObjectA)(JNIEnv*, jclass, jmethodID, void*);                /* 30 */
    jclass      (*GetObjectClass)(JNIEnv*, jobject);                             /* 31 */
    jboolean    (*IsInstanceOf)(JNIEnv*, jobject, jclass);                       /* 32 */
    jmethodID   (*GetMethodID)(JNIEnv*, jclass, const char*, const char*);       /* 33 */

    /* 34-67: Call*Method variants — we just use void* placeholders */
    void *CallObjectMethod;          /* 34 */
    void *CallObjectMethodV;         /* 35 */
    void *CallObjectMethodA;         /* 36 */
    void *CallBooleanMethod;         /* 37 */
    void *CallBooleanMethodV;        /* 38 */
    void *CallBooleanMethodA;        /* 39 */
    void *CallByteMethod;            /* 40 */
    void *CallByteMethodV;           /* 41 */
    void *CallByteMethodA;           /* 42 */
    void *CallCharMethod;            /* 43 */
    void *CallCharMethodV;           /* 44 */
    void *CallCharMethodA;           /* 45 */
    void *CallShortMethod;           /* 46 */
    void *CallShortMethodV;          /* 47 */
    void *CallShortMethodA;          /* 48 */
    void *CallIntMethod;             /* 49 */
    void *CallIntMethodV;            /* 50 */
    void *CallIntMethodA;            /* 51 */
    void *CallLongMethod;            /* 52 */
    void *CallLongMethodV;           /* 53 */
    void *CallLongMethodA;           /* 54 */
    void *CallFloatMethod;           /* 55 */
    void *CallFloatMethodV;          /* 56 */
    void *CallFloatMethodA;          /* 57 */
    void *CallDoubleMethod;          /* 58 */
    void *CallDoubleMethodV;         /* 59 */
    void *CallDoubleMethodA;         /* 60 */
    void *CallVoidMethod;            /* 61 */
    void *CallVoidMethodV;           /* 62 */
    void *CallVoidMethodA;           /* 63 */

    /* 64-96: CallNonvirtual*Method, field access */
    void *CallNonvirtualObjectMethod;  void *CallNonvirtualObjectMethodV;  void *CallNonvirtualObjectMethodA;
    void *CallNonvirtualBooleanMethod; void *CallNonvirtualBooleanMethodV; void *CallNonvirtualBooleanMethodA;
    void *CallNonvirtualByteMethod;    void *CallNonvirtualByteMethodV;    void *CallNonvirtualByteMethodA;
    void *CallNonvirtualCharMethod;    void *CallNonvirtualCharMethodV;    void *CallNonvirtualCharMethodA;
    void *CallNonvirtualShortMethod;   void *CallNonvirtualShortMethodV;   void *CallNonvirtualShortMethodA;
    void *CallNonvirtualIntMethod;     void *CallNonvirtualIntMethodV;     void *CallNonvirtualIntMethodA;
    void *CallNonvirtualLongMethod;    void *CallNonvirtualLongMethodV;    void *CallNonvirtualLongMethodA;
    void *CallNonvirtualFloatMethod;   void *CallNonvirtualFloatMethodV;   void *CallNonvirtualFloatMethodA;
    void *CallNonvirtualDoubleMethod;  void *CallNonvirtualDoubleMethodV;  void *CallNonvirtualDoubleMethodA;
    void *CallNonvirtualVoidMethod;    void *CallNonvirtualVoidMethodV;    void *CallNonvirtualVoidMethodA;

    jfieldID    (*GetFieldID)(JNIEnv*, jclass, const char*, const char*);         /* 94 */

    /* 95-113: Get/Set Field variants */
    void *GetObjectField;  void *GetBooleanField; void *GetByteField;
    void *GetCharField;    void *GetShortField;   void *GetIntField;
    void *GetLongField;    void *GetFloatField;   void *GetDoubleField;
    void *SetObjectField;  void *SetBooleanField; void *SetByteField;
    void *SetCharField;    void *SetShortField;   void *SetIntField;
    void *SetLongField;    void *SetFloatField;   void *SetDoubleField;

    jmethodID   (*GetStaticMethodID)(JNIEnv*, jclass, const char*, const char*); /* 113 */

    /* 114-145: CallStatic*Method variants */
    void *CallStaticObjectMethod;  void *CallStaticObjectMethodV;  void *CallStaticObjectMethodA;
    void *CallStaticBooleanMethod; void *CallStaticBooleanMethodV; void *CallStaticBooleanMethodA;
    void *CallStaticByteMethod;    void *CallStaticByteMethodV;    void *CallStaticByteMethodA;
    void *CallStaticCharMethod;    void *CallStaticCharMethodV;    void *CallStaticCharMethodA;
    void *CallStaticShortMethod;   void *CallStaticShortMethodV;   void *CallStaticShortMethodA;
    void *CallStaticIntMethod;     void *CallStaticIntMethodV;     void *CallStaticIntMethodA;
    void *CallStaticLongMethod;    void *CallStaticLongMethodV;    void *CallStaticLongMethodA;
    void *CallStaticFloatMethod;   void *CallStaticFloatMethodV;   void *CallStaticFloatMethodA;
    void *CallStaticDoubleMethod;  void *CallStaticDoubleMethodV;  void *CallStaticDoubleMethodA;
    void *CallStaticVoidMethod;    void *CallStaticVoidMethodV;    void *CallStaticVoidMethodA;

    jfieldID    (*GetStaticFieldID)(JNIEnv*, jclass, const char*, const char*);

    /* Static field access */
    void *GetStaticObjectField;  void *GetStaticBooleanField; void *GetStaticByteField;
    void *GetStaticCharField;    void *GetStaticShortField;   void *GetStaticIntField;
    void *GetStaticLongField;    void *GetStaticFloatField;   void *GetStaticDoubleField;
    void *SetStaticObjectField;  void *SetStaticBooleanField; void *SetStaticByteField;
    void *SetStaticCharField;    void *SetStaticShortField;   void *SetStaticIntField;
    void *SetStaticLongField;    void *SetStaticFloatField;   void *SetStaticDoubleField;

    jstring     (*NewStringUTF)(JNIEnv*, const char*);                           /* 167 */
    jsize       (*GetStringUTFLength)(JNIEnv*, jstring);                         /* 168 */
    const char* (*GetStringUTFChars)(JNIEnv*, jstring, jboolean*);               /* 169 */
    void        (*ReleaseStringUTFChars)(JNIEnv*, jstring, const char*);         /* 170 */

    jsize       (*GetArrayLength)(JNIEnv*, jarray);                              /* 171 */

    /* Array operations - simplified */
    void *NewObjectArray;    void *GetObjectArrayElement; void *SetObjectArrayElement;
    void *NewBooleanArray;   void *NewByteArray;    void *NewCharArray;
    void *NewShortArray;     void *NewIntArray;     void *NewLongArray;
    void *NewFloatArray;     void *NewDoubleArray;

    /* Get/Release/Set array region functions */
    void *GetBooleanArrayElements; void *GetByteArrayElements; void *GetCharArrayElements;
    void *GetShortArrayElements;   void *GetIntArrayElements;  void *GetLongArrayElements;
    void *GetFloatArrayElements;   void *GetDoubleArrayElements;
    void *ReleaseBooleanArrayElements; void *ReleaseByteArrayElements; void *ReleaseCharArrayElements;
    void *ReleaseShortArrayElements;   void *ReleaseIntArrayElements;  void *ReleaseLongArrayElements;
    void *ReleaseFloatArrayElements;   void *ReleaseDoubleArrayElements;
    void *GetBooleanArrayRegion; void *GetByteArrayRegion; void *GetCharArrayRegion;
    void *GetShortArrayRegion;   void *GetIntArrayRegion;  void *GetLongArrayRegion;
    void *GetFloatArrayRegion;   void *GetDoubleArrayRegion;
    void *SetBooleanArrayRegion; void *SetByteArrayRegion; void *SetCharArrayRegion;
    void *SetShortArrayRegion;   void *SetIntArrayRegion;  void *SetLongArrayRegion;
    void *SetFloatArrayRegion;   void *SetDoubleArrayRegion;

    jint (*RegisterNatives)(JNIEnv*, jclass, const JNINativeMethod*, jint);
    jint (*UnregisterNatives)(JNIEnv*, jclass);
    jint (*MonitorEnter)(JNIEnv*, jobject);
    jint (*MonitorExit)(JNIEnv*, jobject);
    jint (*GetJavaVM)(JNIEnv*, JavaVM**);

    void (*GetStringRegion)(JNIEnv*, jstring, jsize, jsize, jchar*);
    void (*GetStringUTFRegion)(JNIEnv*, jstring, jsize, jsize, char*);

    void *GetPrimitiveArrayCritical;
    void *ReleasePrimitiveArrayCritical;
    void *GetStringCritical;
    void *ReleaseStringCritical;

    jweak  (*NewWeakGlobalRef)(JNIEnv*, jobject);
    void   (*DeleteWeakGlobalRef)(JNIEnv*, jweak);
    jboolean (*ExceptionCheck)(JNIEnv*);

    /* remaining entries... */
    void *NewDirectByteBuffer;
    void *GetDirectBufferAddress;
    void *GetDirectBufferCapacity;
    void *GetObjectRefType;
};

/* JNI Invocation Interface */
struct JNIInvokeInterface_ {
    void *reserved0;
    void *reserved1;
    void *reserved2;
    jint (*DestroyJavaVM)(JavaVM*);
    jint (*AttachCurrentThread)(JavaVM*, JNIEnv**, void*);
    jint (*DetachCurrentThread)(JavaVM*);
    jint (*GetEnv)(JavaVM*, void**, jint);
    jint (*AttachCurrentThreadAsDaemon)(JavaVM*, JNIEnv**, void*);
};

#endif /* FAKE_JNI_H */
