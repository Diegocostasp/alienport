#ifndef BIONIC_SHIMS_H
#define BIONIC_SHIMS_H

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Canário Bionic TLS pad: reserva slot estável em tpidr_el0+0x28 */
extern __attribute__((aligned(16))) _Thread_local char g_bionic_guard_pad[256];

#define BIONIC_FILE_SZ 152
extern unsigned char __sF[BIONIC_FILE_SZ * 3];

extern const unsigned char *_ctype_;

int *bionic_errno(void);
int bionic_system_property_get(const char *name, char *value);
void bionic_set_abort_message(const char *msg);

int __android_log_print(int prio, const char *tag, const char *fmt, ...);
int __android_log_write(int prio, const char *tag, const char *text);
int __android_log_vprint(int prio, const char *tag, const char *fmt, va_list ap);

void __stack_chk_fail(void);

/* Android FORTIFY Shims */
void *bionic_memcpy_chk(void *dest, const void *src, size_t len, size_t destlen);
void *bionic_memmove_chk(void *dest, const void *src, size_t len, size_t destlen);
void *bionic_memset_chk(void *dest, int c, size_t len, size_t destlen);
char *bionic_strcpy_chk(char *dest, const char *src, size_t destlen);
char *bionic_strncpy_chk(char *dest, const char *src, size_t len, size_t destlen);
char *bionic_strncpy_chk2(char *dest, const char *src, size_t len, size_t destlen, size_t srclen);
char *bionic_strchr_chk(const char *s, int c, size_t slen);
size_t bionic_strlen_chk(const char *s, size_t maxlen);
int bionic_vsnprintf_chk(char *s, size_t maxlen, int flag, size_t os, const char *fmt, va_list ap);
int bionic_vsprintf_chk(char *s, int flag, size_t os, const char *fmt, va_list ap);
size_t bionic_fread_chk(void *ptr, size_t size, size_t nmemb, FILE *stream, size_t ptr_size);
size_t bionic_fwrite_chk(const void *ptr, size_t size, size_t nmemb, FILE *stream, size_t ptr_size);
void *bionic_memchr_chk(const void *s, int c, size_t n, size_t s_len);
ssize_t bionic_read_chk(int fd, void *buf, size_t nbytes, size_t buflen);
ssize_t bionic_write_chk(int fd, const void *buf, size_t nbytes, size_t buflen);
int bionic_FD_SET_chk(int fd, void *set, size_t buflen);
int bionic_poll_chk(void *fds, unsigned int nfds, int timeout, size_t buflen);

#ifdef __cplusplus
}
#endif

#endif /* BIONIC_SHIMS_H */
