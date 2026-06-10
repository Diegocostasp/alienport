/*
 * jni_shim.c -- fake JNI environment for Syberia
 *
 * Android JNI works through double-indirection:
 *   JavaVM *vm;   vm->GetEnv(vm, &env, version)
 *   JNIEnv *env;  env->FindClass(env, "com/foo/Bar")
 *
 * Both vm and env are pointers to a pointer to a function table.
 * We create large stub vtables that return 0/NULL for everything,
 * with specific overrides for methods Syberia actually uses.
 */

#include <stdint.h>
#include <string.h>

#include "jni_shim.h"
#include "util.h"

#define JNI_VTABLE_SIZE 512

typedef int jint;

static uintptr_t jni_env_vtable[JNI_VTABLE_SIZE];
static void *jni_env_ptr;

static uintptr_t java_vm_vtable[JNI_VTABLE_SIZE];
static void *java_vm_ptr;

/* ---- Tagged method/field IDs ---- */
enum {
  MID_UNKNOWN = 0,
  MID_GET_STORAGE_DIR,
  MID_GET_PACK_NAME,
  MID_SET_ACTIVITY,
  MID_ERROR_DIALOG,
  MID_GET_CLASS_LOADER,
  MID_LOAD_CLASS,
  MID_GENERIC,
  FID_OBB_VERSIONCODE,
  FID_GENERIC,
};

static long long g_method_tags[16] __attribute__((aligned(8))); /* unique addresses used as method IDs */

/* ---- Configurable package/OBB ---- */
static const char *g_package_name = "com.sigmateam.alienshootermobile.free";
static int g_obb_version = 1;

void jni_shim_set_package(const char *package_name, int obb_version) {
  g_package_name = package_name;
  g_obb_version = obb_version;
}

/* ---- Fake jstring tracking ---- */
/* We return tagged pointers as jstrings and map them to C strings */
#define MAX_JSTRINGS 32
static struct {
  void *handle;
  const char *value;
} g_jstrings[MAX_JSTRINGS];
static int g_jstring_count = 0;

static void *make_jstring(const char *value) {
  static long long jstring_storage[MAX_JSTRINGS] __attribute__((aligned(8)));
  if (g_jstring_count >= MAX_JSTRINGS)
    g_jstring_count = 0; /* wrap around */
  int idx = g_jstring_count++;
  g_jstrings[idx].handle = &jstring_storage[idx];
  g_jstrings[idx].value = value;
  return g_jstrings[idx].handle;
}

static const char *resolve_jstring(void *jstr) {
  for (int i = 0; i < g_jstring_count; i++) {
    if (g_jstrings[i].handle == jstr)
      return g_jstrings[i].value;
  }
  return "";
}

/* ---- Generic stub ---- */
static intptr_t jni_stub_0(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 0); return 0; }
static intptr_t jni_stub_1(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 1); return 0; }
static intptr_t jni_stub_2(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 2); return 0; }
static intptr_t jni_stub_3(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 3); return 0; }
static intptr_t jni_stub_4(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 4); return 0; }
static intptr_t jni_stub_5(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 5); return 0; }
static intptr_t jni_stub_6(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 6); return 0; }
static intptr_t jni_stub_7(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 7); return 0; }
static intptr_t jni_stub_8(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 8); return 0; }
static intptr_t jni_stub_9(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 9); return 0; }
static intptr_t jni_stub_10(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 10); return 0; }
static intptr_t jni_stub_11(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 11); return 0; }
static intptr_t jni_stub_12(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 12); return 0; }
static intptr_t jni_stub_13(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 13); return 0; }
static intptr_t jni_stub_14(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 14); return 0; }
static intptr_t jni_stub_15(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 15); return 0; }
static intptr_t jni_stub_16(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 16); return 0; }
static intptr_t jni_stub_17(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 17); return 0; }
static intptr_t jni_stub_18(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 18); return 0; }
static intptr_t jni_stub_19(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 19); return 0; }
static intptr_t jni_stub_20(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 20); return 0; }
static intptr_t jni_stub_21(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 21); return 0; }
static intptr_t jni_stub_22(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 22); return 0; }
static intptr_t jni_stub_23(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 23); return 0; }
static intptr_t jni_stub_24(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 24); return 0; }
static intptr_t jni_stub_25(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 25); return 0; }
static intptr_t jni_stub_26(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 26); return 0; }
static intptr_t jni_stub_27(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 27); return 0; }
static intptr_t jni_stub_28(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 28); return 0; }
static intptr_t jni_stub_29(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 29); return 0; }
static intptr_t jni_stub_30(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 30); return 0; }
static intptr_t jni_stub_31(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 31); return 0; }
static intptr_t jni_stub_32(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 32); return 0; }
static intptr_t jni_stub_33(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 33); return 0; }
static intptr_t jni_stub_34(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 34); return 0; }
static intptr_t jni_stub_35(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 35); return 0; }
static intptr_t jni_stub_36(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 36); return 0; }
static intptr_t jni_stub_37(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 37); return 0; }
static intptr_t jni_stub_38(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 38); return 0; }
static intptr_t jni_stub_39(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 39); return 0; }
static intptr_t jni_stub_40(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 40); return 0; }
static intptr_t jni_stub_41(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 41); return 0; }
static intptr_t jni_stub_42(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 42); return 0; }
static intptr_t jni_stub_43(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 43); return 0; }
static intptr_t jni_stub_44(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 44); return 0; }
static intptr_t jni_stub_45(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 45); return 0; }
static intptr_t jni_stub_46(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 46); return 0; }
static intptr_t jni_stub_47(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 47); return 0; }
static intptr_t jni_stub_48(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 48); return 0; }
static intptr_t jni_stub_49(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 49); return 0; }
static intptr_t jni_stub_50(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 50); return 0; }
static intptr_t jni_stub_51(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 51); return 0; }
static intptr_t jni_stub_52(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 52); return 0; }
static intptr_t jni_stub_53(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 53); return 0; }
static intptr_t jni_stub_54(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 54); return 0; }
static intptr_t jni_stub_55(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 55); return 0; }
static intptr_t jni_stub_56(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 56); return 0; }
static intptr_t jni_stub_57(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 57); return 0; }
static intptr_t jni_stub_58(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 58); return 0; }
static intptr_t jni_stub_59(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 59); return 0; }
static intptr_t jni_stub_60(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 60); return 0; }
static intptr_t jni_stub_61(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 61); return 0; }
static intptr_t jni_stub_62(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 62); return 0; }
static intptr_t jni_stub_63(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 63); return 0; }
static intptr_t jni_stub_64(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 64); return 0; }
static intptr_t jni_stub_65(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 65); return 0; }
static intptr_t jni_stub_66(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 66); return 0; }
static intptr_t jni_stub_67(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 67); return 0; }
static intptr_t jni_stub_68(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 68); return 0; }
static intptr_t jni_stub_69(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 69); return 0; }
static intptr_t jni_stub_70(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 70); return 0; }
static intptr_t jni_stub_71(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 71); return 0; }
static intptr_t jni_stub_72(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 72); return 0; }
static intptr_t jni_stub_73(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 73); return 0; }
static intptr_t jni_stub_74(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 74); return 0; }
static intptr_t jni_stub_75(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 75); return 0; }
static intptr_t jni_stub_76(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 76); return 0; }
static intptr_t jni_stub_77(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 77); return 0; }
static intptr_t jni_stub_78(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 78); return 0; }
static intptr_t jni_stub_79(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 79); return 0; }
static intptr_t jni_stub_80(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 80); return 0; }
static intptr_t jni_stub_81(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 81); return 0; }
static intptr_t jni_stub_82(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 82); return 0; }
static intptr_t jni_stub_83(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 83); return 0; }
static intptr_t jni_stub_84(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 84); return 0; }
static intptr_t jni_stub_85(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 85); return 0; }
static intptr_t jni_stub_86(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 86); return 0; }
static intptr_t jni_stub_87(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 87); return 0; }
static intptr_t jni_stub_88(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 88); return 0; }
static intptr_t jni_stub_89(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 89); return 0; }
static intptr_t jni_stub_90(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 90); return 0; }
static intptr_t jni_stub_91(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 91); return 0; }
static intptr_t jni_stub_92(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 92); return 0; }
static intptr_t jni_stub_93(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 93); return 0; }
static intptr_t jni_stub_94(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 94); return 0; }
static intptr_t jni_stub_95(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 95); return 0; }
static intptr_t jni_stub_96(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 96); return 0; }
static intptr_t jni_stub_97(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 97); return 0; }
static intptr_t jni_stub_98(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 98); return 0; }
static intptr_t jni_stub_99(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 99); return 0; }
static intptr_t jni_stub_100(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 100); return 0; }
static intptr_t jni_stub_101(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 101); return 0; }
static intptr_t jni_stub_102(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 102); return 0; }
static intptr_t jni_stub_103(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 103); return 0; }
static intptr_t jni_stub_104(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 104); return 0; }
static intptr_t jni_stub_105(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 105); return 0; }
static intptr_t jni_stub_106(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 106); return 0; }
static intptr_t jni_stub_107(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 107); return 0; }
static intptr_t jni_stub_108(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 108); return 0; }
static intptr_t jni_stub_109(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 109); return 0; }
static intptr_t jni_stub_110(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 110); return 0; }
static intptr_t jni_stub_111(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 111); return 0; }
static intptr_t jni_stub_112(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 112); return 0; }
static intptr_t jni_stub_113(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 113); return 0; }
static intptr_t jni_stub_114(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 114); return 0; }
static intptr_t jni_stub_115(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 115); return 0; }
static intptr_t jni_stub_116(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 116); return 0; }
static intptr_t jni_stub_117(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 117); return 0; }
static intptr_t jni_stub_118(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 118); return 0; }
static intptr_t jni_stub_119(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 119); return 0; }
static intptr_t jni_stub_120(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 120); return 0; }
static intptr_t jni_stub_121(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 121); return 0; }
static intptr_t jni_stub_122(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 122); return 0; }
static intptr_t jni_stub_123(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 123); return 0; }
static intptr_t jni_stub_124(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 124); return 0; }
static intptr_t jni_stub_125(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 125); return 0; }
static intptr_t jni_stub_126(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 126); return 0; }
static intptr_t jni_stub_127(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 127); return 0; }
static intptr_t jni_stub_128(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 128); return 0; }
static intptr_t jni_stub_129(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 129); return 0; }
static intptr_t jni_stub_130(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 130); return 0; }
static intptr_t jni_stub_131(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 131); return 0; }
static intptr_t jni_stub_132(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 132); return 0; }
static intptr_t jni_stub_133(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 133); return 0; }
static intptr_t jni_stub_134(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 134); return 0; }
static intptr_t jni_stub_135(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 135); return 0; }
static intptr_t jni_stub_136(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 136); return 0; }
static intptr_t jni_stub_137(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 137); return 0; }
static intptr_t jni_stub_138(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 138); return 0; }
static intptr_t jni_stub_139(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 139); return 0; }
static intptr_t jni_stub_140(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 140); return 0; }
static intptr_t jni_stub_141(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 141); return 0; }
static intptr_t jni_stub_142(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 142); return 0; }
static intptr_t jni_stub_143(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 143); return 0; }
static intptr_t jni_stub_144(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 144); return 0; }
static intptr_t jni_stub_145(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 145); return 0; }
static intptr_t jni_stub_146(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 146); return 0; }
static intptr_t jni_stub_147(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 147); return 0; }
static intptr_t jni_stub_148(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 148); return 0; }
static intptr_t jni_stub_149(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 149); return 0; }
static intptr_t jni_stub_150(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 150); return 0; }
static intptr_t jni_stub_151(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 151); return 0; }
static intptr_t jni_stub_152(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 152); return 0; }
static intptr_t jni_stub_153(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 153); return 0; }
static intptr_t jni_stub_154(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 154); return 0; }
static intptr_t jni_stub_155(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 155); return 0; }
static intptr_t jni_stub_156(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 156); return 0; }
static intptr_t jni_stub_157(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 157); return 0; }
static intptr_t jni_stub_158(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 158); return 0; }
static intptr_t jni_stub_159(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 159); return 0; }
static intptr_t jni_stub_160(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 160); return 0; }
static intptr_t jni_stub_161(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 161); return 0; }
static intptr_t jni_stub_162(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 162); return 0; }
static intptr_t jni_stub_163(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 163); return 0; }
static intptr_t jni_stub_164(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 164); return 0; }
static intptr_t jni_stub_165(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 165); return 0; }
static intptr_t jni_stub_166(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 166); return 0; }
static intptr_t jni_stub_167(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 167); return 0; }
static intptr_t jni_stub_168(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 168); return 0; }
static intptr_t jni_stub_169(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 169); return 0; }
static intptr_t jni_stub_170(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 170); return 0; }
static intptr_t jni_stub_171(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 171); return 0; }
static intptr_t jni_stub_172(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 172); return 0; }
static intptr_t jni_stub_173(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 173); return 0; }
static intptr_t jni_stub_174(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 174); return 0; }
static intptr_t jni_stub_175(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 175); return 0; }
static intptr_t jni_stub_176(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 176); return 0; }
static intptr_t jni_stub_177(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 177); return 0; }
static intptr_t jni_stub_178(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 178); return 0; }
static intptr_t jni_stub_179(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 179); return 0; }
static intptr_t jni_stub_180(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 180); return 0; }
static intptr_t jni_stub_181(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 181); return 0; }
static intptr_t jni_stub_182(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 182); return 0; }
static intptr_t jni_stub_183(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 183); return 0; }
static intptr_t jni_stub_184(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 184); return 0; }
static intptr_t jni_stub_185(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 185); return 0; }
static intptr_t jni_stub_186(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 186); return 0; }
static intptr_t jni_stub_187(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 187); return 0; }
static intptr_t jni_stub_188(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 188); return 0; }
static intptr_t jni_stub_189(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 189); return 0; }
static intptr_t jni_stub_190(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 190); return 0; }
static intptr_t jni_stub_191(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 191); return 0; }
static intptr_t jni_stub_192(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 192); return 0; }
static intptr_t jni_stub_193(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 193); return 0; }
static intptr_t jni_stub_194(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 194); return 0; }
static intptr_t jni_stub_195(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 195); return 0; }
static intptr_t jni_stub_196(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 196); return 0; }
static intptr_t jni_stub_197(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 197); return 0; }
static intptr_t jni_stub_198(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 198); return 0; }
static intptr_t jni_stub_199(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 199); return 0; }
static intptr_t jni_stub_200(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 200); return 0; }
static intptr_t jni_stub_201(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 201); return 0; }
static intptr_t jni_stub_202(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 202); return 0; }
static intptr_t jni_stub_203(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 203); return 0; }
static intptr_t jni_stub_204(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 204); return 0; }
static intptr_t jni_stub_205(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 205); return 0; }
static intptr_t jni_stub_206(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 206); return 0; }
static intptr_t jni_stub_207(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 207); return 0; }
static intptr_t jni_stub_208(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 208); return 0; }
static intptr_t jni_stub_209(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 209); return 0; }
static intptr_t jni_stub_210(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 210); return 0; }
static intptr_t jni_stub_211(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 211); return 0; }
static intptr_t jni_stub_212(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 212); return 0; }
static intptr_t jni_stub_213(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 213); return 0; }
static intptr_t jni_stub_214(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 214); return 0; }
static intptr_t jni_stub_215(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 215); return 0; }
static intptr_t jni_stub_216(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 216); return 0; }
static intptr_t jni_stub_217(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 217); return 0; }
static intptr_t jni_stub_218(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 218); return 0; }
static intptr_t jni_stub_219(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 219); return 0; }
static intptr_t jni_stub_220(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 220); return 0; }
static intptr_t jni_stub_221(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 221); return 0; }
static intptr_t jni_stub_222(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 222); return 0; }
static intptr_t jni_stub_223(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 223); return 0; }
static intptr_t jni_stub_224(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 224); return 0; }
static intptr_t jni_stub_225(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 225); return 0; }
static intptr_t jni_stub_226(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 226); return 0; }
static intptr_t jni_stub_227(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 227); return 0; }
static intptr_t jni_stub_228(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 228); return 0; }
static intptr_t jni_stub_229(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 229); return 0; }
static intptr_t jni_stub_230(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 230); return 0; }
static intptr_t jni_stub_231(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 231); return 0; }
static intptr_t jni_stub_232(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 232); return 0; }
static intptr_t jni_stub_233(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 233); return 0; }
static intptr_t jni_stub_234(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 234); return 0; }
static intptr_t jni_stub_235(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 235); return 0; }
static intptr_t jni_stub_236(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 236); return 0; }
static intptr_t jni_stub_237(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 237); return 0; }
static intptr_t jni_stub_238(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 238); return 0; }
static intptr_t jni_stub_239(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 239); return 0; }
static intptr_t jni_stub_240(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 240); return 0; }
static intptr_t jni_stub_241(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 241); return 0; }
static intptr_t jni_stub_242(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 242); return 0; }
static intptr_t jni_stub_243(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 243); return 0; }
static intptr_t jni_stub_244(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 244); return 0; }
static intptr_t jni_stub_245(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 245); return 0; }
static intptr_t jni_stub_246(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 246); return 0; }
static intptr_t jni_stub_247(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 247); return 0; }
static intptr_t jni_stub_248(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 248); return 0; }
static intptr_t jni_stub_249(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 249); return 0; }
static intptr_t jni_stub_250(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 250); return 0; }
static intptr_t jni_stub_251(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 251); return 0; }
static intptr_t jni_stub_252(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 252); return 0; }
static intptr_t jni_stub_253(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 253); return 0; }
static intptr_t jni_stub_254(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 254); return 0; }
static intptr_t jni_stub_255(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 255); return 0; }
static intptr_t jni_stub_256(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 256); return 0; }
static intptr_t jni_stub_257(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 257); return 0; }
static intptr_t jni_stub_258(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 258); return 0; }
static intptr_t jni_stub_259(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 259); return 0; }
static intptr_t jni_stub_260(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 260); return 0; }
static intptr_t jni_stub_261(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 261); return 0; }
static intptr_t jni_stub_262(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 262); return 0; }
static intptr_t jni_stub_263(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 263); return 0; }
static intptr_t jni_stub_264(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 264); return 0; }
static intptr_t jni_stub_265(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 265); return 0; }
static intptr_t jni_stub_266(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 266); return 0; }
static intptr_t jni_stub_267(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 267); return 0; }
static intptr_t jni_stub_268(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 268); return 0; }
static intptr_t jni_stub_269(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 269); return 0; }
static intptr_t jni_stub_270(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 270); return 0; }
static intptr_t jni_stub_271(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 271); return 0; }
static intptr_t jni_stub_272(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 272); return 0; }
static intptr_t jni_stub_273(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 273); return 0; }
static intptr_t jni_stub_274(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 274); return 0; }
static intptr_t jni_stub_275(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 275); return 0; }
static intptr_t jni_stub_276(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 276); return 0; }
static intptr_t jni_stub_277(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 277); return 0; }
static intptr_t jni_stub_278(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 278); return 0; }
static intptr_t jni_stub_279(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 279); return 0; }
static intptr_t jni_stub_280(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 280); return 0; }
static intptr_t jni_stub_281(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 281); return 0; }
static intptr_t jni_stub_282(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 282); return 0; }
static intptr_t jni_stub_283(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 283); return 0; }
static intptr_t jni_stub_284(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 284); return 0; }
static intptr_t jni_stub_285(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 285); return 0; }
static intptr_t jni_stub_286(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 286); return 0; }
static intptr_t jni_stub_287(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 287); return 0; }
static intptr_t jni_stub_288(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 288); return 0; }
static intptr_t jni_stub_289(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 289); return 0; }
static intptr_t jni_stub_290(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 290); return 0; }
static intptr_t jni_stub_291(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 291); return 0; }
static intptr_t jni_stub_292(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 292); return 0; }
static intptr_t jni_stub_293(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 293); return 0; }
static intptr_t jni_stub_294(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 294); return 0; }
static intptr_t jni_stub_295(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 295); return 0; }
static intptr_t jni_stub_296(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 296); return 0; }
static intptr_t jni_stub_297(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 297); return 0; }
static intptr_t jni_stub_298(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 298); return 0; }
static intptr_t jni_stub_299(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 299); return 0; }
static intptr_t jni_stub_300(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 300); return 0; }
static intptr_t jni_stub_301(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 301); return 0; }
static intptr_t jni_stub_302(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 302); return 0; }
static intptr_t jni_stub_303(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 303); return 0; }
static intptr_t jni_stub_304(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 304); return 0; }
static intptr_t jni_stub_305(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 305); return 0; }
static intptr_t jni_stub_306(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 306); return 0; }
static intptr_t jni_stub_307(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 307); return 0; }
static intptr_t jni_stub_308(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 308); return 0; }
static intptr_t jni_stub_309(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 309); return 0; }
static intptr_t jni_stub_310(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 310); return 0; }
static intptr_t jni_stub_311(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 311); return 0; }
static intptr_t jni_stub_312(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 312); return 0; }
static intptr_t jni_stub_313(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 313); return 0; }
static intptr_t jni_stub_314(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 314); return 0; }
static intptr_t jni_stub_315(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 315); return 0; }
static intptr_t jni_stub_316(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 316); return 0; }
static intptr_t jni_stub_317(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 317); return 0; }
static intptr_t jni_stub_318(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 318); return 0; }
static intptr_t jni_stub_319(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 319); return 0; }
static intptr_t jni_stub_320(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 320); return 0; }
static intptr_t jni_stub_321(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 321); return 0; }
static intptr_t jni_stub_322(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 322); return 0; }
static intptr_t jni_stub_323(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 323); return 0; }
static intptr_t jni_stub_324(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 324); return 0; }
static intptr_t jni_stub_325(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 325); return 0; }
static intptr_t jni_stub_326(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 326); return 0; }
static intptr_t jni_stub_327(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 327); return 0; }
static intptr_t jni_stub_328(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 328); return 0; }
static intptr_t jni_stub_329(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 329); return 0; }
static intptr_t jni_stub_330(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 330); return 0; }
static intptr_t jni_stub_331(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 331); return 0; }
static intptr_t jni_stub_332(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 332); return 0; }
static intptr_t jni_stub_333(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 333); return 0; }
static intptr_t jni_stub_334(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 334); return 0; }
static intptr_t jni_stub_335(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 335); return 0; }
static intptr_t jni_stub_336(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 336); return 0; }
static intptr_t jni_stub_337(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 337); return 0; }
static intptr_t jni_stub_338(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 338); return 0; }
static intptr_t jni_stub_339(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 339); return 0; }
static intptr_t jni_stub_340(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 340); return 0; }
static intptr_t jni_stub_341(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 341); return 0; }
static intptr_t jni_stub_342(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 342); return 0; }
static intptr_t jni_stub_343(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 343); return 0; }
static intptr_t jni_stub_344(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 344); return 0; }
static intptr_t jni_stub_345(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 345); return 0; }
static intptr_t jni_stub_346(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 346); return 0; }
static intptr_t jni_stub_347(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 347); return 0; }
static intptr_t jni_stub_348(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 348); return 0; }
static intptr_t jni_stub_349(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 349); return 0; }
static intptr_t jni_stub_350(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 350); return 0; }
static intptr_t jni_stub_351(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 351); return 0; }
static intptr_t jni_stub_352(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 352); return 0; }
static intptr_t jni_stub_353(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 353); return 0; }
static intptr_t jni_stub_354(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 354); return 0; }
static intptr_t jni_stub_355(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 355); return 0; }
static intptr_t jni_stub_356(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 356); return 0; }
static intptr_t jni_stub_357(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 357); return 0; }
static intptr_t jni_stub_358(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 358); return 0; }
static intptr_t jni_stub_359(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 359); return 0; }
static intptr_t jni_stub_360(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 360); return 0; }
static intptr_t jni_stub_361(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 361); return 0; }
static intptr_t jni_stub_362(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 362); return 0; }
static intptr_t jni_stub_363(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 363); return 0; }
static intptr_t jni_stub_364(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 364); return 0; }
static intptr_t jni_stub_365(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 365); return 0; }
static intptr_t jni_stub_366(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 366); return 0; }
static intptr_t jni_stub_367(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 367); return 0; }
static intptr_t jni_stub_368(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 368); return 0; }
static intptr_t jni_stub_369(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 369); return 0; }
static intptr_t jni_stub_370(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 370); return 0; }
static intptr_t jni_stub_371(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 371); return 0; }
static intptr_t jni_stub_372(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 372); return 0; }
static intptr_t jni_stub_373(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 373); return 0; }
static intptr_t jni_stub_374(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 374); return 0; }
static intptr_t jni_stub_375(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 375); return 0; }
static intptr_t jni_stub_376(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 376); return 0; }
static intptr_t jni_stub_377(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 377); return 0; }
static intptr_t jni_stub_378(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 378); return 0; }
static intptr_t jni_stub_379(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 379); return 0; }
static intptr_t jni_stub_380(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 380); return 0; }
static intptr_t jni_stub_381(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 381); return 0; }
static intptr_t jni_stub_382(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 382); return 0; }
static intptr_t jni_stub_383(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 383); return 0; }
static intptr_t jni_stub_384(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 384); return 0; }
static intptr_t jni_stub_385(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 385); return 0; }
static intptr_t jni_stub_386(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 386); return 0; }
static intptr_t jni_stub_387(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 387); return 0; }
static intptr_t jni_stub_388(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 388); return 0; }
static intptr_t jni_stub_389(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 389); return 0; }
static intptr_t jni_stub_390(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 390); return 0; }
static intptr_t jni_stub_391(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 391); return 0; }
static intptr_t jni_stub_392(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 392); return 0; }
static intptr_t jni_stub_393(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 393); return 0; }
static intptr_t jni_stub_394(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 394); return 0; }
static intptr_t jni_stub_395(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 395); return 0; }
static intptr_t jni_stub_396(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 396); return 0; }
static intptr_t jni_stub_397(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 397); return 0; }
static intptr_t jni_stub_398(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 398); return 0; }
static intptr_t jni_stub_399(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 399); return 0; }
static intptr_t jni_stub_400(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 400); return 0; }
static intptr_t jni_stub_401(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 401); return 0; }
static intptr_t jni_stub_402(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 402); return 0; }
static intptr_t jni_stub_403(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 403); return 0; }
static intptr_t jni_stub_404(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 404); return 0; }
static intptr_t jni_stub_405(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 405); return 0; }
static intptr_t jni_stub_406(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 406); return 0; }
static intptr_t jni_stub_407(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 407); return 0; }
static intptr_t jni_stub_408(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 408); return 0; }
static intptr_t jni_stub_409(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 409); return 0; }
static intptr_t jni_stub_410(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 410); return 0; }
static intptr_t jni_stub_411(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 411); return 0; }
static intptr_t jni_stub_412(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 412); return 0; }
static intptr_t jni_stub_413(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 413); return 0; }
static intptr_t jni_stub_414(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 414); return 0; }
static intptr_t jni_stub_415(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 415); return 0; }
static intptr_t jni_stub_416(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 416); return 0; }
static intptr_t jni_stub_417(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 417); return 0; }
static intptr_t jni_stub_418(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 418); return 0; }
static intptr_t jni_stub_419(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 419); return 0; }
static intptr_t jni_stub_420(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 420); return 0; }
static intptr_t jni_stub_421(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 421); return 0; }
static intptr_t jni_stub_422(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 422); return 0; }
static intptr_t jni_stub_423(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 423); return 0; }
static intptr_t jni_stub_424(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 424); return 0; }
static intptr_t jni_stub_425(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 425); return 0; }
static intptr_t jni_stub_426(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 426); return 0; }
static intptr_t jni_stub_427(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 427); return 0; }
static intptr_t jni_stub_428(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 428); return 0; }
static intptr_t jni_stub_429(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 429); return 0; }
static intptr_t jni_stub_430(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 430); return 0; }
static intptr_t jni_stub_431(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 431); return 0; }
static intptr_t jni_stub_432(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 432); return 0; }
static intptr_t jni_stub_433(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 433); return 0; }
static intptr_t jni_stub_434(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 434); return 0; }
static intptr_t jni_stub_435(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 435); return 0; }
static intptr_t jni_stub_436(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 436); return 0; }
static intptr_t jni_stub_437(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 437); return 0; }
static intptr_t jni_stub_438(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 438); return 0; }
static intptr_t jni_stub_439(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 439); return 0; }
static intptr_t jni_stub_440(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 440); return 0; }
static intptr_t jni_stub_441(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 441); return 0; }
static intptr_t jni_stub_442(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 442); return 0; }
static intptr_t jni_stub_443(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 443); return 0; }
static intptr_t jni_stub_444(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 444); return 0; }
static intptr_t jni_stub_445(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 445); return 0; }
static intptr_t jni_stub_446(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 446); return 0; }
static intptr_t jni_stub_447(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 447); return 0; }
static intptr_t jni_stub_448(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 448); return 0; }
static intptr_t jni_stub_449(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 449); return 0; }
static intptr_t jni_stub_450(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 450); return 0; }
static intptr_t jni_stub_451(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 451); return 0; }
static intptr_t jni_stub_452(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 452); return 0; }
static intptr_t jni_stub_453(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 453); return 0; }
static intptr_t jni_stub_454(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 454); return 0; }
static intptr_t jni_stub_455(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 455); return 0; }
static intptr_t jni_stub_456(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 456); return 0; }
static intptr_t jni_stub_457(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 457); return 0; }
static intptr_t jni_stub_458(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 458); return 0; }
static intptr_t jni_stub_459(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 459); return 0; }
static intptr_t jni_stub_460(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 460); return 0; }
static intptr_t jni_stub_461(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 461); return 0; }
static intptr_t jni_stub_462(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 462); return 0; }
static intptr_t jni_stub_463(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 463); return 0; }
static intptr_t jni_stub_464(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 464); return 0; }
static intptr_t jni_stub_465(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 465); return 0; }
static intptr_t jni_stub_466(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 466); return 0; }
static intptr_t jni_stub_467(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 467); return 0; }
static intptr_t jni_stub_468(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 468); return 0; }
static intptr_t jni_stub_469(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 469); return 0; }
static intptr_t jni_stub_470(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 470); return 0; }
static intptr_t jni_stub_471(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 471); return 0; }
static intptr_t jni_stub_472(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 472); return 0; }
static intptr_t jni_stub_473(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 473); return 0; }
static intptr_t jni_stub_474(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 474); return 0; }
static intptr_t jni_stub_475(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 475); return 0; }
static intptr_t jni_stub_476(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 476); return 0; }
static intptr_t jni_stub_477(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 477); return 0; }
static intptr_t jni_stub_478(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 478); return 0; }
static intptr_t jni_stub_479(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 479); return 0; }
static intptr_t jni_stub_480(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 480); return 0; }
static intptr_t jni_stub_481(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 481); return 0; }
static intptr_t jni_stub_482(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 482); return 0; }
static intptr_t jni_stub_483(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 483); return 0; }
static intptr_t jni_stub_484(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 484); return 0; }
static intptr_t jni_stub_485(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 485); return 0; }
static intptr_t jni_stub_486(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 486); return 0; }
static intptr_t jni_stub_487(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 487); return 0; }
static intptr_t jni_stub_488(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 488); return 0; }
static intptr_t jni_stub_489(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 489); return 0; }
static intptr_t jni_stub_490(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 490); return 0; }
static intptr_t jni_stub_491(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 491); return 0; }
static intptr_t jni_stub_492(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 492); return 0; }
static intptr_t jni_stub_493(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 493); return 0; }
static intptr_t jni_stub_494(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 494); return 0; }
static intptr_t jni_stub_495(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 495); return 0; }
static intptr_t jni_stub_496(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 496); return 0; }
static intptr_t jni_stub_497(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 497); return 0; }
static intptr_t jni_stub_498(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 498); return 0; }
static intptr_t jni_stub_499(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 499); return 0; }
static intptr_t jni_stub_500(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 500); return 0; }
static intptr_t jni_stub_501(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 501); return 0; }
static intptr_t jni_stub_502(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 502); return 0; }
static intptr_t jni_stub_503(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 503); return 0; }
static intptr_t jni_stub_504(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 504); return 0; }
static intptr_t jni_stub_505(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 505); return 0; }
static intptr_t jni_stub_506(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 506); return 0; }
static intptr_t jni_stub_507(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 507); return 0; }
static intptr_t jni_stub_508(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 508); return 0; }
static intptr_t jni_stub_509(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 509); return 0; }
static intptr_t jni_stub_510(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 510); return 0; }
static intptr_t jni_stub_511(void) { debugPrintf("jni_shim: STUB CALLED %d\n", 511); return 0; }


/* ---- JNIEnv functions ---- */

static jint jni_GetVersion(void *env) {
  (void)env;
  return 0x00010006;
}

static void *jni_FindClass(void *env, const char *name) {
  (void)env;
  debugPrintf("jni_shim: FindClass(%s)\n", name);
  static long long fake_class __attribute__((aligned(8)));
  return &fake_class;
}

static void *jni_GetMethodID(void *env, void *clazz, const char *name,
                             const char *sig) {
  (void)env;
  (void)clazz;
  debugPrintf("jni_shim: GetMethodID(%s, %s)\n", name, sig);
  if (strcmp(name, "getClassLoader") == 0)
    return &g_method_tags[MID_GET_CLASS_LOADER];
  if (strcmp(name, "loadClass") == 0)
    return &g_method_tags[MID_LOAD_CLASS];
  return &g_method_tags[MID_GENERIC];
}

static void *jni_GetStaticMethodID(void *env, void *clazz, const char *name,
                                   const char *sig) {
  (void)env;
  (void)clazz;
  debugPrintf("jni_shim: GetStaticMethodID(%s, %s)\n", name, sig);
  if (strcmp(name, "getStorageDir") == 0)
    return &g_method_tags[MID_GET_STORAGE_DIR];
  if (strcmp(name, "getPackName") == 0)
    return &g_method_tags[MID_GET_PACK_NAME];
  if (strcmp(name, "setActivity") == 0)
    return &g_method_tags[MID_SET_ACTIVITY];
  if (strcmp(name, "errorDialog") == 0)
    return &g_method_tags[MID_ERROR_DIALOG];
  return &g_method_tags[MID_GENERIC];
}

static void *jni_GetFieldID(void *env, void *clazz, const char *name,
                            const char *sig) {
  (void)env;
  (void)clazz;
  debugPrintf("jni_shim: GetFieldID(%s, %s)\n", name, sig);
  return &g_method_tags[FID_GENERIC];
}

static void *jni_GetStaticFieldID(void *env, void *clazz, const char *name,
                                  const char *sig) {
  (void)env;
  (void)clazz;
  debugPrintf("jni_shim: GetStaticFieldID(%s, %s)\n", name, sig);
  if (strcmp(name, "OBB_VERSIONCODE") == 0)
    return &g_method_tags[FID_OBB_VERSIONCODE];
  return &g_method_tags[FID_GENERIC];
}

/* CallObjectMethod (index 36) - variadic */
static void *jni_CallObjectMethod(void *env, void *obj, void *methodID, ...) {
  (void)env;
  (void)obj;
  debugPrintf("jni_shim: CallObjectMethod(mid=%p)\n", methodID);
  static long long fake_obj __attribute__((aligned(8)));
  return &fake_obj;
}

/* CallBooleanMethod (index 49) */
static unsigned char jni_CallBooleanMethod(void *env, void *obj,
                                           void *methodID, ...) {
  (void)env;
  (void)obj;
  (void)methodID;
  return 0;
}

/* CallIntMethod (index 61) */
static jint jni_CallIntMethod(void *env, void *obj, void *methodID, ...) {
  (void)env;
  (void)obj;
  (void)methodID;
  return 0;
}

/* CallVoidMethod (index 94) */
static void jni_CallVoidMethod(void *env, void *obj, void *methodID, ...) {
  (void)env;
  (void)obj;
  (void)methodID;
}

/* CallStaticObjectMethod (index 113) */
static void *jni_CallStaticObjectMethod(void *env, void *clazz,
                                        void *methodID, ...) {
  (void)env;
  (void)clazz;

  if (methodID == &g_method_tags[MID_GET_STORAGE_DIR]) {
    debugPrintf("jni_shim: CallStaticObjectMethod -> getStorageDir = \".\"\n");
    return make_jstring(".");
  }
  if (methodID == &g_method_tags[MID_GET_PACK_NAME]) {
    debugPrintf(
        "jni_shim: CallStaticObjectMethod -> getPackName = \"%s\"\n",
        g_package_name);
    return make_jstring(g_package_name);
  }

  debugPrintf("jni_shim: CallStaticObjectMethod(mid=%p) -> NULL\n", methodID);
  static long long fake_result __attribute__((aligned(8)));
  return &fake_result;
}

/* CallStaticBooleanMethod (index 124) */
static unsigned char jni_CallStaticBooleanMethod(void *env, void *clazz,
                                                 void *methodID, ...) {
  (void)env;
  (void)clazz;
  debugPrintf("jni_shim: CallStaticBooleanMethod(mid=%p) -> 1\n", methodID);
  // Return true for hasTouchScreen — prevents game from managing
  // Shield gamepad button layouts that don't exist in the OBB.
  return 1;
}

/* CallStaticIntMethod (index 136) */
static jint jni_CallStaticIntMethod(void *env, void *clazz, void *methodID,
                                    ...) {
  (void)env;
  (void)clazz;
  (void)methodID;
  debugPrintf("jni_shim: CallStaticIntMethod(mid=%p)
", methodID);
  return 0;
}

/* CallStaticVoidMethod (index 145) */
static void jni_CallStaticVoidMethod(void *env, void *clazz, void *methodID,
                                     ...) {
  (void)env;
  (void)clazz;
  debugPrintf("jni_shim: CallStaticVoidMethod(mid=%p)\n", methodID);
}

/* GetStaticIntField (index 155) */
static jint jni_GetStaticIntField(void *env, void *clazz, void *fieldID) {
  (void)env;
  (void)clazz;

  if (fieldID == &g_method_tags[FID_OBB_VERSIONCODE]) {
    debugPrintf("jni_shim: GetStaticIntField -> OBB_VERSIONCODE = %d\n",
                g_obb_version);
    return g_obb_version;
  }
  debugPrintf("jni_shim: GetStaticIntField(fid=%p) -> 0\n", fieldID);
  return 0;
}

/* GetStaticObjectField (index 156) */
static void *jni_GetStaticObjectField(void *env, void *clazz, void *fieldID) {
  (void)env;
  (void)clazz;
  (void)fieldID;
  debugPrintf("jni_shim: GetStaticObjectField -> NULL\n");
  static long long fake __attribute__((aligned(8)));
  return &fake;
}

/* NewStringUTF (index 167) */
static void *jni_NewStringUTF(void *env, const char *str) {
  (void)env;
  debugPrintf("jni_shim: NewStringUTF(%s)\n", str ? str : "(null)");
  return make_jstring(str ? str : "");
}

/* GetStringUTFLength (index 168) */
static jint jni_GetStringUTFLength(void *env, void *jstr) {
  (void)env;
  const char *s = resolve_jstring(jstr);
  return (jint)strlen(s);
}

/* GetStringUTFChars (index 169) */
static const char *jni_GetStringUTFChars(void *env, void *jstr,
                                         void *isCopy) {
  (void)env;
  (void)isCopy;
  const char *s = resolve_jstring(jstr);
  debugPrintf("jni_shim: GetStringUTFChars -> \"%s\"\n", s);
  return s;
}

/* ReleaseStringUTFChars (index 170) */
static void jni_ReleaseStringUTFChars(void *env, void *jstr,
                                      const char *chars) {
  (void)env;
  (void)jstr;
  (void)chars;
}

/* Ref management */
static void *jni_NewGlobalRef(void *env, void *obj) {
  (void)env;
  return obj;
}
static void *jni_NewLocalRef(void *env, void *obj) {
  (void)env;
  return obj;
}
static void jni_DeleteGlobalRef(void *env, void *obj) {
  (void)env;
  (void)obj;
}
static void jni_DeleteLocalRef(void *env, void *obj) {
  (void)env;
  (void)obj;
}
static void *jni_GetObjectClass(void *env, void *obj) {
  (void)env;
  (void)obj;
  static long long fake_obj_class __attribute__((aligned(8)));
  return &fake_obj_class;
}

/* Exception handling */
static unsigned char jni_ExceptionCheck(void *env) {
  (void)env;
  return 0;
}
static void jni_ExceptionClear(void *env) { (void)env; }
static void *jni_ExceptionOccurred(void *env) {
  (void)env;
  return 0;
}

/* Array */
static jint jni_GetArrayLength(void *env, void *array) {
  (void)env;
  (void)array;
  return 0;
}

/* ---- JavaVM functions ---- */

static jint vm_DestroyJavaVM(void *vm) {
  (void)vm;
  return 0;
}

static jint vm_AttachCurrentThread(void *vm, void **penv, void *args) {
  (void)vm;
  (void)args;
  debugPrintf("jni_shim: AttachCurrentThread\n");
  if (penv)
    *penv = &jni_env_ptr;
  return 0;
}

static jint vm_DetachCurrentThread(void *vm) {
  (void)vm;
  return 0;
}

static jint vm_GetEnv(void *vm, void **penv, jint version) {
  (void)vm;
  (void)version;
  debugPrintf("jni_shim: GetEnv(version=0x%x)\n", version);
  if (penv)
    *penv = &jni_env_ptr;
  return 0;
}

static jint vm_AttachCurrentThreadAsDaemon(void *vm, void **penv, void *args) {
  (void)vm;
  (void)args;
  if (penv)
    *penv = &jni_env_ptr;
  return 0;
}

/* ---- Init ---- */

void jni_shim_init(void **out_vm, void **out_env) {
  static const uintptr_t stubs[] = {(uintptr_t)jni_stub_0, (uintptr_t)jni_stub_1, (uintptr_t)jni_stub_2, (uintptr_t)jni_stub_3, (uintptr_t)jni_stub_4, (uintptr_t)jni_stub_5, (uintptr_t)jni_stub_6, (uintptr_t)jni_stub_7, (uintptr_t)jni_stub_8, (uintptr_t)jni_stub_9, (uintptr_t)jni_stub_10, (uintptr_t)jni_stub_11, (uintptr_t)jni_stub_12, (uintptr_t)jni_stub_13, (uintptr_t)jni_stub_14, (uintptr_t)jni_stub_15, (uintptr_t)jni_stub_16, (uintptr_t)jni_stub_17, (uintptr_t)jni_stub_18, (uintptr_t)jni_stub_19, (uintptr_t)jni_stub_20, (uintptr_t)jni_stub_21, (uintptr_t)jni_stub_22, (uintptr_t)jni_stub_23, (uintptr_t)jni_stub_24, (uintptr_t)jni_stub_25, (uintptr_t)jni_stub_26, (uintptr_t)jni_stub_27, (uintptr_t)jni_stub_28, (uintptr_t)jni_stub_29, (uintptr_t)jni_stub_30, (uintptr_t)jni_stub_31, (uintptr_t)jni_stub_32, (uintptr_t)jni_stub_33, (uintptr_t)jni_stub_34, (uintptr_t)jni_stub_35, (uintptr_t)jni_stub_36, (uintptr_t)jni_stub_37, (uintptr_t)jni_stub_38, (uintptr_t)jni_stub_39, (uintptr_t)jni_stub_40, (uintptr_t)jni_stub_41, (uintptr_t)jni_stub_42, (uintptr_t)jni_stub_43, (uintptr_t)jni_stub_44, (uintptr_t)jni_stub_45, (uintptr_t)jni_stub_46, (uintptr_t)jni_stub_47, (uintptr_t)jni_stub_48, (uintptr_t)jni_stub_49, (uintptr_t)jni_stub_50, (uintptr_t)jni_stub_51, (uintptr_t)jni_stub_52, (uintptr_t)jni_stub_53, (uintptr_t)jni_stub_54, (uintptr_t)jni_stub_55, (uintptr_t)jni_stub_56, (uintptr_t)jni_stub_57, (uintptr_t)jni_stub_58, (uintptr_t)jni_stub_59, (uintptr_t)jni_stub_60, (uintptr_t)jni_stub_61, (uintptr_t)jni_stub_62, (uintptr_t)jni_stub_63, (uintptr_t)jni_stub_64, (uintptr_t)jni_stub_65, (uintptr_t)jni_stub_66, (uintptr_t)jni_stub_67, (uintptr_t)jni_stub_68, (uintptr_t)jni_stub_69, (uintptr_t)jni_stub_70, (uintptr_t)jni_stub_71, (uintptr_t)jni_stub_72, (uintptr_t)jni_stub_73, (uintptr_t)jni_stub_74, (uintptr_t)jni_stub_75, (uintptr_t)jni_stub_76, (uintptr_t)jni_stub_77, (uintptr_t)jni_stub_78, (uintptr_t)jni_stub_79, (uintptr_t)jni_stub_80, (uintptr_t)jni_stub_81, (uintptr_t)jni_stub_82, (uintptr_t)jni_stub_83, (uintptr_t)jni_stub_84, (uintptr_t)jni_stub_85, (uintptr_t)jni_stub_86, (uintptr_t)jni_stub_87, (uintptr_t)jni_stub_88, (uintptr_t)jni_stub_89, (uintptr_t)jni_stub_90, (uintptr_t)jni_stub_91, (uintptr_t)jni_stub_92, (uintptr_t)jni_stub_93, (uintptr_t)jni_stub_94, (uintptr_t)jni_stub_95, (uintptr_t)jni_stub_96, (uintptr_t)jni_stub_97, (uintptr_t)jni_stub_98, (uintptr_t)jni_stub_99, (uintptr_t)jni_stub_100, (uintptr_t)jni_stub_101, (uintptr_t)jni_stub_102, (uintptr_t)jni_stub_103, (uintptr_t)jni_stub_104, (uintptr_t)jni_stub_105, (uintptr_t)jni_stub_106, (uintptr_t)jni_stub_107, (uintptr_t)jni_stub_108, (uintptr_t)jni_stub_109, (uintptr_t)jni_stub_110, (uintptr_t)jni_stub_111, (uintptr_t)jni_stub_112, (uintptr_t)jni_stub_113, (uintptr_t)jni_stub_114, (uintptr_t)jni_stub_115, (uintptr_t)jni_stub_116, (uintptr_t)jni_stub_117, (uintptr_t)jni_stub_118, (uintptr_t)jni_stub_119, (uintptr_t)jni_stub_120, (uintptr_t)jni_stub_121, (uintptr_t)jni_stub_122, (uintptr_t)jni_stub_123, (uintptr_t)jni_stub_124, (uintptr_t)jni_stub_125, (uintptr_t)jni_stub_126, (uintptr_t)jni_stub_127, (uintptr_t)jni_stub_128, (uintptr_t)jni_stub_129, (uintptr_t)jni_stub_130, (uintptr_t)jni_stub_131, (uintptr_t)jni_stub_132, (uintptr_t)jni_stub_133, (uintptr_t)jni_stub_134, (uintptr_t)jni_stub_135, (uintptr_t)jni_stub_136, (uintptr_t)jni_stub_137, (uintptr_t)jni_stub_138, (uintptr_t)jni_stub_139, (uintptr_t)jni_stub_140, (uintptr_t)jni_stub_141, (uintptr_t)jni_stub_142, (uintptr_t)jni_stub_143, (uintptr_t)jni_stub_144, (uintptr_t)jni_stub_145, (uintptr_t)jni_stub_146, (uintptr_t)jni_stub_147, (uintptr_t)jni_stub_148, (uintptr_t)jni_stub_149, (uintptr_t)jni_stub_150, (uintptr_t)jni_stub_151, (uintptr_t)jni_stub_152, (uintptr_t)jni_stub_153, (uintptr_t)jni_stub_154, (uintptr_t)jni_stub_155, (uintptr_t)jni_stub_156, (uintptr_t)jni_stub_157, (uintptr_t)jni_stub_158, (uintptr_t)jni_stub_159, (uintptr_t)jni_stub_160, (uintptr_t)jni_stub_161, (uintptr_t)jni_stub_162, (uintptr_t)jni_stub_163, (uintptr_t)jni_stub_164, (uintptr_t)jni_stub_165, (uintptr_t)jni_stub_166, (uintptr_t)jni_stub_167, (uintptr_t)jni_stub_168, (uintptr_t)jni_stub_169, (uintptr_t)jni_stub_170, (uintptr_t)jni_stub_171, (uintptr_t)jni_stub_172, (uintptr_t)jni_stub_173, (uintptr_t)jni_stub_174, (uintptr_t)jni_stub_175, (uintptr_t)jni_stub_176, (uintptr_t)jni_stub_177, (uintptr_t)jni_stub_178, (uintptr_t)jni_stub_179, (uintptr_t)jni_stub_180, (uintptr_t)jni_stub_181, (uintptr_t)jni_stub_182, (uintptr_t)jni_stub_183, (uintptr_t)jni_stub_184, (uintptr_t)jni_stub_185, (uintptr_t)jni_stub_186, (uintptr_t)jni_stub_187, (uintptr_t)jni_stub_188, (uintptr_t)jni_stub_189, (uintptr_t)jni_stub_190, (uintptr_t)jni_stub_191, (uintptr_t)jni_stub_192, (uintptr_t)jni_stub_193, (uintptr_t)jni_stub_194, (uintptr_t)jni_stub_195, (uintptr_t)jni_stub_196, (uintptr_t)jni_stub_197, (uintptr_t)jni_stub_198, (uintptr_t)jni_stub_199, (uintptr_t)jni_stub_200, (uintptr_t)jni_stub_201, (uintptr_t)jni_stub_202, (uintptr_t)jni_stub_203, (uintptr_t)jni_stub_204, (uintptr_t)jni_stub_205, (uintptr_t)jni_stub_206, (uintptr_t)jni_stub_207, (uintptr_t)jni_stub_208, (uintptr_t)jni_stub_209, (uintptr_t)jni_stub_210, (uintptr_t)jni_stub_211, (uintptr_t)jni_stub_212, (uintptr_t)jni_stub_213, (uintptr_t)jni_stub_214, (uintptr_t)jni_stub_215, (uintptr_t)jni_stub_216, (uintptr_t)jni_stub_217, (uintptr_t)jni_stub_218, (uintptr_t)jni_stub_219, (uintptr_t)jni_stub_220, (uintptr_t)jni_stub_221, (uintptr_t)jni_stub_222, (uintptr_t)jni_stub_223, (uintptr_t)jni_stub_224, (uintptr_t)jni_stub_225, (uintptr_t)jni_stub_226, (uintptr_t)jni_stub_227, (uintptr_t)jni_stub_228, (uintptr_t)jni_stub_229, (uintptr_t)jni_stub_230, (uintptr_t)jni_stub_231, (uintptr_t)jni_stub_232, (uintptr_t)jni_stub_233, (uintptr_t)jni_stub_234, (uintptr_t)jni_stub_235, (uintptr_t)jni_stub_236, (uintptr_t)jni_stub_237, (uintptr_t)jni_stub_238, (uintptr_t)jni_stub_239, (uintptr_t)jni_stub_240, (uintptr_t)jni_stub_241, (uintptr_t)jni_stub_242, (uintptr_t)jni_stub_243, (uintptr_t)jni_stub_244, (uintptr_t)jni_stub_245, (uintptr_t)jni_stub_246, (uintptr_t)jni_stub_247, (uintptr_t)jni_stub_248, (uintptr_t)jni_stub_249, (uintptr_t)jni_stub_250, (uintptr_t)jni_stub_251, (uintptr_t)jni_stub_252, (uintptr_t)jni_stub_253, (uintptr_t)jni_stub_254, (uintptr_t)jni_stub_255, (uintptr_t)jni_stub_256, (uintptr_t)jni_stub_257, (uintptr_t)jni_stub_258, (uintptr_t)jni_stub_259, (uintptr_t)jni_stub_260, (uintptr_t)jni_stub_261, (uintptr_t)jni_stub_262, (uintptr_t)jni_stub_263, (uintptr_t)jni_stub_264, (uintptr_t)jni_stub_265, (uintptr_t)jni_stub_266, (uintptr_t)jni_stub_267, (uintptr_t)jni_stub_268, (uintptr_t)jni_stub_269, (uintptr_t)jni_stub_270, (uintptr_t)jni_stub_271, (uintptr_t)jni_stub_272, (uintptr_t)jni_stub_273, (uintptr_t)jni_stub_274, (uintptr_t)jni_stub_275, (uintptr_t)jni_stub_276, (uintptr_t)jni_stub_277, (uintptr_t)jni_stub_278, (uintptr_t)jni_stub_279, (uintptr_t)jni_stub_280, (uintptr_t)jni_stub_281, (uintptr_t)jni_stub_282, (uintptr_t)jni_stub_283, (uintptr_t)jni_stub_284, (uintptr_t)jni_stub_285, (uintptr_t)jni_stub_286, (uintptr_t)jni_stub_287, (uintptr_t)jni_stub_288, (uintptr_t)jni_stub_289, (uintptr_t)jni_stub_290, (uintptr_t)jni_stub_291, (uintptr_t)jni_stub_292, (uintptr_t)jni_stub_293, (uintptr_t)jni_stub_294, (uintptr_t)jni_stub_295, (uintptr_t)jni_stub_296, (uintptr_t)jni_stub_297, (uintptr_t)jni_stub_298, (uintptr_t)jni_stub_299, (uintptr_t)jni_stub_300, (uintptr_t)jni_stub_301, (uintptr_t)jni_stub_302, (uintptr_t)jni_stub_303, (uintptr_t)jni_stub_304, (uintptr_t)jni_stub_305, (uintptr_t)jni_stub_306, (uintptr_t)jni_stub_307, (uintptr_t)jni_stub_308, (uintptr_t)jni_stub_309, (uintptr_t)jni_stub_310, (uintptr_t)jni_stub_311, (uintptr_t)jni_stub_312, (uintptr_t)jni_stub_313, (uintptr_t)jni_stub_314, (uintptr_t)jni_stub_315, (uintptr_t)jni_stub_316, (uintptr_t)jni_stub_317, (uintptr_t)jni_stub_318, (uintptr_t)jni_stub_319, (uintptr_t)jni_stub_320, (uintptr_t)jni_stub_321, (uintptr_t)jni_stub_322, (uintptr_t)jni_stub_323, (uintptr_t)jni_stub_324, (uintptr_t)jni_stub_325, (uintptr_t)jni_stub_326, (uintptr_t)jni_stub_327, (uintptr_t)jni_stub_328, (uintptr_t)jni_stub_329, (uintptr_t)jni_stub_330, (uintptr_t)jni_stub_331, (uintptr_t)jni_stub_332, (uintptr_t)jni_stub_333, (uintptr_t)jni_stub_334, (uintptr_t)jni_stub_335, (uintptr_t)jni_stub_336, (uintptr_t)jni_stub_337, (uintptr_t)jni_stub_338, (uintptr_t)jni_stub_339, (uintptr_t)jni_stub_340, (uintptr_t)jni_stub_341, (uintptr_t)jni_stub_342, (uintptr_t)jni_stub_343, (uintptr_t)jni_stub_344, (uintptr_t)jni_stub_345, (uintptr_t)jni_stub_346, (uintptr_t)jni_stub_347, (uintptr_t)jni_stub_348, (uintptr_t)jni_stub_349, (uintptr_t)jni_stub_350, (uintptr_t)jni_stub_351, (uintptr_t)jni_stub_352, (uintptr_t)jni_stub_353, (uintptr_t)jni_stub_354, (uintptr_t)jni_stub_355, (uintptr_t)jni_stub_356, (uintptr_t)jni_stub_357, (uintptr_t)jni_stub_358, (uintptr_t)jni_stub_359, (uintptr_t)jni_stub_360, (uintptr_t)jni_stub_361, (uintptr_t)jni_stub_362, (uintptr_t)jni_stub_363, (uintptr_t)jni_stub_364, (uintptr_t)jni_stub_365, (uintptr_t)jni_stub_366, (uintptr_t)jni_stub_367, (uintptr_t)jni_stub_368, (uintptr_t)jni_stub_369, (uintptr_t)jni_stub_370, (uintptr_t)jni_stub_371, (uintptr_t)jni_stub_372, (uintptr_t)jni_stub_373, (uintptr_t)jni_stub_374, (uintptr_t)jni_stub_375, (uintptr_t)jni_stub_376, (uintptr_t)jni_stub_377, (uintptr_t)jni_stub_378, (uintptr_t)jni_stub_379, (uintptr_t)jni_stub_380, (uintptr_t)jni_stub_381, (uintptr_t)jni_stub_382, (uintptr_t)jni_stub_383, (uintptr_t)jni_stub_384, (uintptr_t)jni_stub_385, (uintptr_t)jni_stub_386, (uintptr_t)jni_stub_387, (uintptr_t)jni_stub_388, (uintptr_t)jni_stub_389, (uintptr_t)jni_stub_390, (uintptr_t)jni_stub_391, (uintptr_t)jni_stub_392, (uintptr_t)jni_stub_393, (uintptr_t)jni_stub_394, (uintptr_t)jni_stub_395, (uintptr_t)jni_stub_396, (uintptr_t)jni_stub_397, (uintptr_t)jni_stub_398, (uintptr_t)jni_stub_399, (uintptr_t)jni_stub_400, (uintptr_t)jni_stub_401, (uintptr_t)jni_stub_402, (uintptr_t)jni_stub_403, (uintptr_t)jni_stub_404, (uintptr_t)jni_stub_405, (uintptr_t)jni_stub_406, (uintptr_t)jni_stub_407, (uintptr_t)jni_stub_408, (uintptr_t)jni_stub_409, (uintptr_t)jni_stub_410, (uintptr_t)jni_stub_411, (uintptr_t)jni_stub_412, (uintptr_t)jni_stub_413, (uintptr_t)jni_stub_414, (uintptr_t)jni_stub_415, (uintptr_t)jni_stub_416, (uintptr_t)jni_stub_417, (uintptr_t)jni_stub_418, (uintptr_t)jni_stub_419, (uintptr_t)jni_stub_420, (uintptr_t)jni_stub_421, (uintptr_t)jni_stub_422, (uintptr_t)jni_stub_423, (uintptr_t)jni_stub_424, (uintptr_t)jni_stub_425, (uintptr_t)jni_stub_426, (uintptr_t)jni_stub_427, (uintptr_t)jni_stub_428, (uintptr_t)jni_stub_429, (uintptr_t)jni_stub_430, (uintptr_t)jni_stub_431, (uintptr_t)jni_stub_432, (uintptr_t)jni_stub_433, (uintptr_t)jni_stub_434, (uintptr_t)jni_stub_435, (uintptr_t)jni_stub_436, (uintptr_t)jni_stub_437, (uintptr_t)jni_stub_438, (uintptr_t)jni_stub_439, (uintptr_t)jni_stub_440, (uintptr_t)jni_stub_441, (uintptr_t)jni_stub_442, (uintptr_t)jni_stub_443, (uintptr_t)jni_stub_444, (uintptr_t)jni_stub_445, (uintptr_t)jni_stub_446, (uintptr_t)jni_stub_447, (uintptr_t)jni_stub_448, (uintptr_t)jni_stub_449, (uintptr_t)jni_stub_450, (uintptr_t)jni_stub_451, (uintptr_t)jni_stub_452, (uintptr_t)jni_stub_453, (uintptr_t)jni_stub_454, (uintptr_t)jni_stub_455, (uintptr_t)jni_stub_456, (uintptr_t)jni_stub_457, (uintptr_t)jni_stub_458, (uintptr_t)jni_stub_459, (uintptr_t)jni_stub_460, (uintptr_t)jni_stub_461, (uintptr_t)jni_stub_462, (uintptr_t)jni_stub_463, (uintptr_t)jni_stub_464, (uintptr_t)jni_stub_465, (uintptr_t)jni_stub_466, (uintptr_t)jni_stub_467, (uintptr_t)jni_stub_468, (uintptr_t)jni_stub_469, (uintptr_t)jni_stub_470, (uintptr_t)jni_stub_471, (uintptr_t)jni_stub_472, (uintptr_t)jni_stub_473, (uintptr_t)jni_stub_474, (uintptr_t)jni_stub_475, (uintptr_t)jni_stub_476, (uintptr_t)jni_stub_477, (uintptr_t)jni_stub_478, (uintptr_t)jni_stub_479, (uintptr_t)jni_stub_480, (uintptr_t)jni_stub_481, (uintptr_t)jni_stub_482, (uintptr_t)jni_stub_483, (uintptr_t)jni_stub_484, (uintptr_t)jni_stub_485, (uintptr_t)jni_stub_486, (uintptr_t)jni_stub_487, (uintptr_t)jni_stub_488, (uintptr_t)jni_stub_489, (uintptr_t)jni_stub_490, (uintptr_t)jni_stub_491, (uintptr_t)jni_stub_492, (uintptr_t)jni_stub_493, (uintptr_t)jni_stub_494, (uintptr_t)jni_stub_495, (uintptr_t)jni_stub_496, (uintptr_t)jni_stub_497, (uintptr_t)jni_stub_498, (uintptr_t)jni_stub_499, (uintptr_t)jni_stub_500, (uintptr_t)jni_stub_501, (uintptr_t)jni_stub_502, (uintptr_t)jni_stub_503, (uintptr_t)jni_stub_504, (uintptr_t)jni_stub_505, (uintptr_t)jni_stub_506, (uintptr_t)jni_stub_507, (uintptr_t)jni_stub_508, (uintptr_t)jni_stub_509, (uintptr_t)jni_stub_510, (uintptr_t)jni_stub_511};
  for (int i = 0; i < JNI_VTABLE_SIZE; i++) {
    jni_env_vtable[i] = stubs[i];
    java_vm_vtable[i] = stubs[i];
  }

  /*
   * JNIEnv vtable indices from Android NDK jni.h.
   * C++ wrappers in the .so call the *V (va_list) variants,
   * so we must set both the variadic and V slots.
   *
   *   0-3:   reserved
   *   4:     GetVersion
   *   6:     FindClass
   *  15:     ExceptionOccurred
   *  17:     ExceptionClear
   *  21:     NewGlobalRef
   *  22:     DeleteGlobalRef
   *  23:     DeleteLocalRef
   *  25:     NewLocalRef
   *  31:     GetObjectClass
   *  33:     GetMethodID
   *  34/35:  CallObjectMethod / V
   *  37/38:  CallBooleanMethod / V
   *  49/50:  CallIntMethod / V
   *  61/62:  CallVoidMethod / V
   *  94:     GetFieldID
   * 113:     GetStaticMethodID
   * 114/115: CallStaticObjectMethod / V
   * 117/118: CallStaticBooleanMethod / V
   * 129/130: CallStaticIntMethod / V
   * 141/142: CallStaticVoidMethod / V
   * 144:     GetStaticFieldID
   * 145:     GetStaticObjectField
   * 150:     GetStaticIntField
   * 167:     NewStringUTF
   * 168:     GetStringUTFLength
   * 169:     GetStringUTFChars
   * 170:     ReleaseStringUTFChars
   * 171:     GetArrayLength
   * 205:     ExceptionCheck
   */
  jni_env_vtable[4] = (uintptr_t)jni_GetVersion;
  jni_env_vtable[6] = (uintptr_t)jni_FindClass;
  jni_env_vtable[15] = (uintptr_t)jni_ExceptionOccurred;
  jni_env_vtable[17] = (uintptr_t)jni_ExceptionClear;
  jni_env_vtable[21] = (uintptr_t)jni_NewGlobalRef;
  jni_env_vtable[22] = (uintptr_t)jni_DeleteGlobalRef;
  jni_env_vtable[23] = (uintptr_t)jni_DeleteLocalRef;
  jni_env_vtable[25] = (uintptr_t)jni_NewLocalRef;
  jni_env_vtable[31] = (uintptr_t)jni_GetObjectClass;
  jni_env_vtable[33] = (uintptr_t)jni_GetMethodID;
  jni_env_vtable[34] = (uintptr_t)jni_CallObjectMethod;
  jni_env_vtable[35] = (uintptr_t)jni_CallObjectMethod;    /* V variant */
  jni_env_vtable[37] = (uintptr_t)jni_CallBooleanMethod;
  jni_env_vtable[38] = (uintptr_t)jni_CallBooleanMethod;   /* V */
  jni_env_vtable[49] = (uintptr_t)jni_CallIntMethod;
  jni_env_vtable[50] = (uintptr_t)jni_CallIntMethod;       /* V */
  jni_env_vtable[61] = (uintptr_t)jni_CallVoidMethod;
  jni_env_vtable[62] = (uintptr_t)jni_CallVoidMethod;      /* V */
  jni_env_vtable[94] = (uintptr_t)jni_GetFieldID;
  jni_env_vtable[113] = (uintptr_t)jni_GetStaticMethodID;
  jni_env_vtable[114] = (uintptr_t)jni_CallStaticObjectMethod;
  jni_env_vtable[115] = (uintptr_t)jni_CallStaticObjectMethod; /* V */
  jni_env_vtable[117] = (uintptr_t)jni_CallStaticBooleanMethod;
  jni_env_vtable[118] = (uintptr_t)jni_CallStaticBooleanMethod; /* V */
  jni_env_vtable[129] = (uintptr_t)jni_CallStaticIntMethod;
  jni_env_vtable[130] = (uintptr_t)jni_CallStaticIntMethod; /* V */
  jni_env_vtable[141] = (uintptr_t)jni_CallStaticVoidMethod;
  jni_env_vtable[142] = (uintptr_t)jni_CallStaticVoidMethod; /* V */
  jni_env_vtable[144] = (uintptr_t)jni_GetStaticFieldID;
  jni_env_vtable[145] = (uintptr_t)jni_GetStaticObjectField;
  jni_env_vtable[150] = (uintptr_t)jni_GetStaticIntField;
  jni_env_vtable[167] = (uintptr_t)jni_NewStringUTF;
  jni_env_vtable[168] = (uintptr_t)jni_GetStringUTFLength;
  jni_env_vtable[169] = (uintptr_t)jni_GetStringUTFChars;
  jni_env_vtable[170] = (uintptr_t)jni_ReleaseStringUTFChars;
  jni_env_vtable[171] = (uintptr_t)jni_GetArrayLength;
  jni_env_vtable[205] = (uintptr_t)jni_ExceptionCheck;

  jni_env_ptr = jni_env_vtable;

  /* JavaVM vtable */
  java_vm_vtable[3] = (uintptr_t)vm_DestroyJavaVM;
  java_vm_vtable[4] = (uintptr_t)vm_AttachCurrentThread;
  java_vm_vtable[5] = (uintptr_t)vm_DetachCurrentThread;
  java_vm_vtable[6] = (uintptr_t)vm_GetEnv;
  java_vm_vtable[7] = (uintptr_t)vm_AttachCurrentThreadAsDaemon;

  java_vm_ptr = java_vm_vtable;

  if (out_vm)
    *out_vm = &java_vm_ptr;
  if (out_env)
    *out_env = &jni_env_ptr;

  debugPrintf("jni_shim: Initialized (vm=%p, env=%p)\n", &java_vm_ptr,
              &jni_env_ptr);
}
