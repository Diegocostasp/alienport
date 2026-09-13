#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "bionic_shims.h"
#include "so_util.h"
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

__attribute__((used, aligned(16))) _Thread_local char g_bionic_guard_pad[256];

unsigned char __sF[BIONIC_FILE_SZ * 3];

#define GD_CT_U 0x01
#define GD_CT_L 0x02
#define GD_CT_N 0x04
#define GD_CT_S 0x08
#define GD_CT_P 0x10
#define GD_CT_C 0x20
#define GD_CT_X 0x40
#define GD_CT_B 0x80

static unsigned char bionic_ctype_table[257];
const unsigned char *_ctype_ = bionic_ctype_table;

__attribute__((constructor)) static void bionic_ctype_init(void) {
  for (int c = 0; c < 256; c++) {
    unsigned char v = 0;
    if (c >= 'A' && c <= 'Z') v |= GD_CT_U;
    if (c >= 'a' && c <= 'z') v |= GD_CT_L;
    if (c >= '0' && c <= '9') v |= GD_CT_N;
    if (c == ' ' || (c >= '\t' && c <= '\r')) v |= GD_CT_S;
    if (c == ' ' || c == '\t') v |= GD_CT_B;
    if (c < 0x20 || c == 0x7f) v |= GD_CT_C;
    if ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) v |= GD_CT_X;
    if (c > 0x20 && c < 0x7f && !(v & (GD_CT_U | GD_CT_L | GD_CT_N))) v |= GD_CT_P;
    bionic_ctype_table[c + 1] = v;
  }
  bionic_ctype_table[0] = 0;
}

int *bionic_errno(void) {
  return &errno;
}

int bionic_system_property_get(const char *name, char *value) {
  if (!name || !value) return 0;
  if (!strcmp(name, "ro.build.version.sdk")) {
    strcpy(value, "28");
    return (int)strlen(value);
  }
  if (!strcmp(name, "ro.product.model")) {
    strcpy(value, "R36S Handheld");
    return (int)strlen(value);
  }
  if (!strcmp(name, "ro.product.manufacturer")) {
    strcpy(value, "PortMaster");
    return (int)strlen(value);
  }
  value[0] = '\0';
  return 0;
}

void bionic_set_abort_message(const char *msg) {
  if (msg) fprintf(stderr, "[bionic-abort] %s\n", msg);
}

void *bionic_malloc(size_t size) {
  return malloc(size);
}

void *bionic_calloc(size_t n, size_t size) {
  return calloc(n, size);
}

void *bionic_realloc(void *ptr, size_t size) {
  if (!ptr) return malloc(size);
  if ((uintptr_t)ptr < 0x10000 || ((uintptr_t)ptr & 0xF) != 0) {
    return malloc(size);
  }
  if (load_base && (uintptr_t)ptr >= (uintptr_t)load_base &&
      (uintptr_t)ptr < (uintptr_t)load_base + load_size) {
    void *new_ptr = malloc(size);
    return new_ptr;
  }
  return realloc(ptr, size);
}

#include <sys/mman.h>
#include <pthread.h>

void bionic_free(void *ptr) {
  if (!ptr) return;

  // 1. Ignore low sentinel / unmapped / NULL-page addresses
  if ((uintptr_t)ptr < 0x10000) return;

  // 2. Ignore pointers inside the loaded .so memory (text/rodata/data/bss)
  if (load_base && (uintptr_t)ptr >= (uintptr_t)load_base &&
      (uintptr_t)ptr < (uintptr_t)load_base + load_size) {
    return;
  }

  // 3. glibc malloc chunks on AArch64 are strictly 16-byte aligned.
  // Passing an unaligned pointer to glibc free() immediately triggers "free(): invalid pointer"!
  if (((uintptr_t)ptr & 0xF) != 0) {
    return;
  }

  // 4. Verify pointer's header page is mapped in virtual memory
  unsigned char vec[1];
  void *page = (void *)(((uintptr_t)ptr - 16) & ~4095UL);
  if (mincore(page, 4096, vec) != 0) {
    return;
  }

  // 5. Glibc malloc chunk header check
  // For any allocated chunk, the size field at ((size_t*)ptr)[-1] must be >= 32
  // and multiple of 16 (on aarch64), and reasonably bounded.
  size_t chunk_header = ((size_t *)ptr)[-1];
  size_t chunk_size = chunk_header & ~0x7UL;
  if (chunk_size < 32 || (chunk_size & 0xF) != 0 || chunk_size > 0x1000000000ULL) {
    fprintf(stderr, "[bionic_free] Ignored invalid chunk ptr=%p (header=0x%zx)\n", ptr, chunk_header);
    return;
  }

  free(ptr);
}

void bionic_delete(void *ptr) {
  bionic_free(ptr);
}

void bionic_delete_sized(void *ptr, size_t sz) {
  (void)sz;
  bionic_free(ptr);
}

static const char lvl[] = "??VDIWEF";

int __android_log_print(int prio, const char *tag, const char *fmt, ...) {
  va_list ap;
  char buf[2048];
  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);
  fprintf(stderr, "[%c/%s] %s\n", lvl[(prio >= 0 && prio < 8) ? prio : 0], tag ? tag : "?", buf);
  return 0;
}

int __android_log_write(int prio, const char *tag, const char *text) {
  fprintf(stderr, "[%c/%s] %s\n", lvl[(prio >= 0 && prio < 8) ? prio : 0], tag ? tag : "?", text ? text : "");
  return 0;
}

int __android_log_vprint(int prio, const char *tag, const char *fmt, va_list ap) {
  char buf[2048];
  vsnprintf(buf, sizeof(buf), fmt, ap);
  return __android_log_write(prio, tag, buf);
}

void *bionic_memcpy_chk(void *dest, const void *src, size_t len, size_t destlen) {
  (void)destlen;
  return memcpy(dest, src, len);
}

void *bionic_memmove_chk(void *dest, const void *src, size_t len, size_t destlen) {
  (void)destlen;
  return memmove(dest, src, len);
}

void *bionic_memset_chk(void *dest, int c, size_t len, size_t destlen) {
  (void)destlen;
  return memset(dest, c, len);
}

char *bionic_strcpy_chk(char *dest, const char *src, size_t destlen) {
  (void)destlen;
  return strcpy(dest, src);
}

char *bionic_strncpy_chk(char *dest, const char *src, size_t len, size_t destlen) {
  (void)destlen;
  return strncpy(dest, src, len);
}

char *bionic_strncpy_chk2(char *dest, const char *src, size_t len, size_t destlen, size_t srclen) {
  (void)destlen; (void)srclen;
  return strncpy(dest, src, len);
}

char *bionic_strchr_chk(const char *s, int c, size_t slen) {
  (void)slen;
  return strchr(s, c);
}

size_t bionic_strlen_chk(const char *s, size_t maxlen) {
  (void)maxlen;
  return strlen(s);
}

int bionic_vsnprintf_chk(char *s, size_t maxlen, int flag, size_t os, const char *fmt, va_list ap) {
  (void)flag; (void)os;
  return vsnprintf(s, maxlen, fmt, ap);
}

int bionic_vsprintf_chk(char *s, int flag, size_t os, const char *fmt, va_list ap) {
  (void)flag; (void)os;
  return vsprintf(s, fmt, ap);
}

size_t bionic_fread_chk(void *ptr, size_t size, size_t nmemb, FILE *stream, size_t ptr_size) {
  (void)ptr_size;
  return fread(ptr, size, nmemb, stream);
}

size_t bionic_fwrite_chk(const void *ptr, size_t size, size_t nmemb, FILE *stream, size_t ptr_size) {
  (void)ptr_size;
  return fwrite(ptr, size, nmemb, stream);
}

void *bionic_memchr_chk(const void *s, int c, size_t n, size_t s_len) {
  (void)s_len;
  return memchr(s, c, n);
}

ssize_t bionic_read_chk(int fd, void *buf, size_t nbytes, size_t buflen) {
  (void)buflen;
  return read(fd, buf, nbytes);
}

ssize_t bionic_write_chk(int fd, const void *buf, size_t nbytes, size_t buflen) {
  (void)buflen;
  return write(fd, buf, nbytes);
}

int bionic_FD_SET_chk(int fd, void *set, size_t buflen) {
  (void)buflen;
  FD_SET(fd, (fd_set *)set);
  return 0;
}

int bionic_poll_chk(void *fds, unsigned int nfds, int timeout, size_t buflen) {
  (void)buflen;
  return poll((struct pollfd *)fds, nfds, timeout);
}

void bionic_stack_chk_fail(void) {
  void *caller = __builtin_return_address(0);
  uintptr_t lr = (uintptr_t)caller;
  fprintf(stderr, "\n==================== STACK SMASHING DETECTADO ====================\n");
  fprintf(stderr, "[bionic] __stack_chk_fail() chamado diretamente por: %p", caller);
  if (load_base && lr >= (uintptr_t)load_base && lr < (uintptr_t)load_base + load_size) {
    fprintf(stderr, " (libalien_shooter.so offset: +0x%lx)\n", (unsigned long)(lr - (uintptr_t)load_base));
  } else {
    fprintf(stderr, "\n");
  }

  uintptr_t fp = (uintptr_t)__builtin_frame_address(0);
  fprintf(stderr, "[bionic] Call stack unwind:\n");
  for (int i = 0; i < 24 && fp && (fp & 7) == 0; i++) {
    uintptr_t next_fp = *(uintptr_t *)fp;
    uintptr_t ret = *(uintptr_t *)(fp + 8);
    fprintf(stderr, "  #%02d fp=%p lr=%p", i, (void *)fp, (void *)ret);
    if (load_base && ret >= (uintptr_t)load_base && ret < (uintptr_t)load_base + load_size) {
      fprintf(stderr, " (libalien_shooter.so +0x%lx)", (unsigned long)(ret - (uintptr_t)load_base));
    }
    fprintf(stderr, "\n");
    if (next_fp <= fp || (next_fp - fp) > 0x200000) break;
    fp = next_fp;
  }
  fprintf(stderr, "===================================================================\n");
  fflush(stderr);
  abort();
}
