#ifndef SO_UTIL_H
#define SO_UTIL_H

#include <stddef.h>
#include <stdint.h>

typedef struct {
  char *symbol;
  uintptr_t func;
} DynLibFunction;

int so_load(const char *filename, void *base, size_t max_size);
int so_relocate(void);
int so_resolve(DynLibFunction *funcs, int num_funcs, int taint_missing_imports);
void so_execute_init_array(void);
uintptr_t so_find_addr(const char *symbol);
uintptr_t so_find_addr_rx(const char *symbol);
void so_flush_caches(void);
void so_make_text_writable(void);
void so_make_text_executable(void);
void so_finalize(void);

#define ALIGN_MEM(x, align) (((x) + ((align) - 1)) & ~((align) - 1))

#endif
