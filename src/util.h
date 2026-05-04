/*
 * util.h -- Utility functions
 */

#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdarg.h>

static inline void debugPrintf(const char *fmt, ...) {
    va_list list;
    va_start(list, fmt);
    vprintf(fmt, list);
    va_end(list);
    fflush(stdout);
}

static inline int ret0(void) { return 0; }
static inline int ret1(void) { return 1; }
static inline int retm1(void) { return -1; }

#endif /* UTIL_H */
