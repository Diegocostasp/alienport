#ifndef ANDROID_BRIDGE_H
#define ANDROID_BRIDGE_H

#include <stdarg.h>
#include <stdio.h>
#include <jni.h>
#include "so_util.h"

void jni_init();
extern DynLibFunction android_imports[];
extern int num_android_imports;

#endif
