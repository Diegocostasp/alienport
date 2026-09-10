/*
 * so_util.c -- Carregador ELF ARM64 para libalien_shooter.so
 *
 * Baseado na implementação comprovada de Beach Buggy Racing / reVC / Syberia.
 */

#include "so_util.h"
#include <assert.h>
#include <dlfcn.h>
#include <elf.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#ifndef EM_AARCH64
#define EM_AARCH64 183
#endif

void *text_base, *text_virtbase;
size_t text_size;

void *data_base, *data_virtbase;
size_t data_size;

static void *load_base, *load_virtbase;
static size_t load_size;

static void *so_base;

static Elf64_Ehdr *elf_hdr;
static Elf64_Phdr *prog_hdr;
static Elf64_Shdr *sec_hdr;
static Elf64_Sym *syms;
static int num_syms;

static char *shstrtab;
static char *dynstrtab;

void hook_arm64(uintptr_t addr, uintptr_t dst) {
  if (addr == 0) return;
  uint32_t *hook = (uint32_t *)addr;
  hook[0] = 0x58000051u; // LDR X17, #0x8
  hook[1] = 0xd61f0220u; // BR X17
  *(uint64_t *)(hook + 2) = dst;
  __builtin___clear_cache((char *)hook, (char *)hook + 16);
}

void so_make_text_writable(void) {
  const size_t text_asize = ALIGN_MEM(text_size, 0x1000);
  mprotect(text_virtbase, text_asize, PROT_READ | PROT_WRITE | PROT_EXEC);
}

void so_make_text_executable(void) {
  const size_t text_asize = ALIGN_MEM(text_size, 0x1000);
  mprotect(text_virtbase, text_asize, PROT_READ | PROT_EXEC);
}

void so_flush_caches(void) {
  __builtin___clear_cache((char *)load_virtbase, (char *)load_virtbase + load_size);
}

void so_free_temp(void) {
  if (so_base) {
    free(so_base);
    so_base = NULL;
  }
}

int so_load(const char *filename, void *base, size_t max_size) {
  int res = 0;
  size_t so_size = 0;

  printf("[so_util] Opening %s\n", filename);
  FILE *fd = fopen(filename, "rb");
  if (!fd) {
    fprintf(stderr, "[so_util] Failed to open %s\n", filename);
    return -1;
  }

  fseek(fd, 0, SEEK_END);
  so_size = ftell(fd);
  fseek(fd, 0, SEEK_SET);

  so_base = malloc(so_size);
  if (!so_base) {
    fclose(fd);
    return -2;
  }

  if (fread(so_base, so_size, 1, fd) != 1) {
    fclose(fd);
    free(so_base);
    return -3;
  }
  fclose(fd);

  if (memcmp(so_base, ELFMAG, SELFMAG) != 0) {
    fprintf(stderr, "[so_util] Not a valid ELF file\n");
    res = -1;
    goto err_free_so;
  }

  elf_hdr = (Elf64_Ehdr *)so_base;
  if (elf_hdr->e_ident[EI_CLASS] != ELFCLASS64 || elf_hdr->e_machine != EM_AARCH64) {
    fprintf(stderr, "[so_util] Not an AArch64 ELF file\n");
    res = -1;
    goto err_free_so;
  }

  prog_hdr = (Elf64_Phdr *)((uintptr_t)so_base + elf_hdr->e_phoff);
  sec_hdr = (Elf64_Shdr *)((uintptr_t)so_base + elf_hdr->e_shoff);
  shstrtab = (char *)((uintptr_t)so_base + sec_hdr[elf_hdr->e_shstrndx].sh_offset);

  size_t max_end = 0;
  int exec_seg = -1;
  for (int i = 0; i < elf_hdr->e_phnum; i++) {
    if (prog_hdr[i].p_type == PT_LOAD) {
      size_t end = prog_hdr[i].p_vaddr + prog_hdr[i].p_memsz;
      if (end > max_end) max_end = end;
      if ((prog_hdr[i].p_flags & PF_X) == PF_X) exec_seg = i;
    }
  }

  if (exec_seg < 0) {
    res = -1;
    goto err_free_so;
  }

  load_size = ALIGN_MEM(max_end, 0x1000);
  if (base == NULL) {
    base = mmap(NULL, load_size, PROT_READ | PROT_WRITE | PROT_EXEC,
                MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (base == MAP_FAILED) {
      res = -4;
      goto err_free_so;
    }
  }

  load_base = base;
  memset(load_base, 0, load_size);
  load_virtbase = load_base;

  for (int i = 0; i < elf_hdr->e_phnum; i++) {
    if (prog_hdr[i].p_type != PT_LOAD) continue;
    memcpy((void *)((uintptr_t)load_base + prog_hdr[i].p_vaddr),
           (void *)((uintptr_t)so_base + prog_hdr[i].p_offset),
           prog_hdr[i].p_filesz);
  }

  text_size = prog_hdr[exec_seg].p_memsz;
  text_base = (void *)((uintptr_t)load_base + prog_hdr[exec_seg].p_vaddr);
  text_virtbase = text_base;
  data_base = load_base;
  data_size = load_size;
  data_virtbase = load_base;

  for (int i = 0; i < elf_hdr->e_shnum; i++) {
    char *sh_name = shstrtab + sec_hdr[i].sh_name;
    if (strcmp(sh_name, ".dynsym") == 0) {
      syms = (Elf64_Sym *)((uintptr_t)load_base + sec_hdr[i].sh_addr);
      num_syms = sec_hdr[i].sh_size / sizeof(Elf64_Sym);
    } else if (strcmp(sh_name, ".dynstr") == 0) {
      dynstrtab = (char *)((uintptr_t)load_base + sec_hdr[i].sh_addr);
    }
  }

  printf("[so_util] Successfully loaded %s at %p (size: %zu MB)\n",
         filename, load_base, load_size / (1024 * 1024));
  return 0;

err_free_so:
  free(so_base);
  so_base = NULL;
  return res;
}

int so_relocate(void) {
  for (int i = 0; i < elf_hdr->e_shnum; i++) {
    char *sh_name = shstrtab + sec_hdr[i].sh_name;
    if (strcmp(sh_name, ".rela.dyn") == 0 || strcmp(sh_name, ".rela.plt") == 0) {
      Elf64_Rela *relas = (Elf64_Rela *)((uintptr_t)load_base + sec_hdr[i].sh_addr);
      int num_relas = sec_hdr[i].sh_size / sizeof(Elf64_Rela);

      for (int j = 0; j < num_relas; j++) {
        Elf64_Rela *rela = &relas[j];
        uint32_t type = ELF64_R_TYPE(rela->r_info);
        uintptr_t *ptr = (uintptr_t *)((uintptr_t)load_base + rela->r_offset);

        if (type == R_AARCH64_RELATIVE) {
          *ptr = (uintptr_t)load_base + rela->r_addend;
        }
      }
    }
  }
  return 0;
}

int so_resolve(DynLibFunction *funcs, int num_funcs, int taint_missing_imports) {
  int missing = 0;
  for (int i = 0; i < elf_hdr->e_shnum; i++) {
    char *sh_name = shstrtab + sec_hdr[i].sh_name;
    if (strcmp(sh_name, ".rela.dyn") == 0 || strcmp(sh_name, ".rela.plt") == 0) {
      Elf64_Rela *relas = (Elf64_Rela *)((uintptr_t)load_base + sec_hdr[i].sh_addr);
      int num_relas = sec_hdr[i].sh_size / sizeof(Elf64_Rela);

      for (int j = 0; j < num_relas; j++) {
        Elf64_Rela *rela = &relas[j];
        uint32_t type = ELF64_R_TYPE(rela->r_info);
        uint32_t sym_idx = ELF64_R_SYM(rela->r_info);
        uintptr_t *ptr = (uintptr_t *)((uintptr_t)load_base + rela->r_offset);

        if (type == R_AARCH64_GLOB_DAT || type == R_AARCH64_JUMP_SLOT || type == R_AARCH64_ABS64) {
          if (sym_idx != 0) {
            char *sym_name = dynstrtab + syms[sym_idx].st_name;
            uintptr_t resolved = 0;

            for (int k = 0; k < num_funcs; k++) {
              if (strcmp(sym_name, funcs[k].symbol) == 0) {
                resolved = funcs[k].func;
                break;
              }
            }

            if (!resolved) {
              resolved = (uintptr_t)dlsym(RTLD_DEFAULT, sym_name);
            }

            if (resolved) {
              *ptr = resolved + rela->r_addend;
            } else {
              missing++;
              // printf("[so_util] Missing symbol: %s\n", sym_name);
            }
          }
        }
      }
    }
  }
  printf("[so_util] Resolved symbols (%d unmapped)\n", missing);
  return 0;
}

void so_execute_init_array(void) {
  for (int i = 0; i < elf_hdr->e_shnum; i++) {
    char *sh_name = shstrtab + sec_hdr[i].sh_name;
    if (strcmp(sh_name, ".init_array") == 0) {
      uintptr_t *inits = (uintptr_t *)((uintptr_t)load_base + sec_hdr[i].sh_addr);
      int count = sec_hdr[i].sh_size / sizeof(uintptr_t);
      for (int j = 0; j < count; j++) {
        if (inits[j] && inits[j] != (uintptr_t)-1) {
          void (*fn)(void) = (void (*)(void))inits[j];
          fn();
        }
      }
    }
  }
}

uintptr_t so_find_addr(const char *symbol) {
  if (!syms || !dynstrtab) return 0;
  for (int i = 0; i < num_syms; i++) {
    if (syms[i].st_shndx != SHN_UNDEF) {
      char *name = dynstrtab + syms[i].st_name;
      if (strcmp(name, symbol) == 0) {
        return (uintptr_t)load_base + syms[i].st_value;
      }
    }
  }
  return 0;
}

uintptr_t so_find_rel_addr_safe(const char *symbol) {
  for (int i = 0; i < elf_hdr->e_shnum; i++) {
    char *sh_name = shstrtab + sec_hdr[i].sh_name;
    if (strcmp(sh_name, ".rela.dyn") == 0 || strcmp(sh_name, ".rela.plt") == 0) {
      Elf64_Rela *relas = (Elf64_Rela *)((uintptr_t)load_base + sec_hdr[i].sh_addr);
      int num_relas = sec_hdr[i].sh_size / sizeof(Elf64_Rela);
      for (int j = 0; j < num_relas; j++) {
        uint32_t sym_idx = ELF64_R_SYM(relas[j].r_info);
        if (sym_idx != 0) {
          char *name = dynstrtab + syms[sym_idx].st_name;
          if (strcmp(name, symbol) == 0) {
            return (uintptr_t)load_base + relas[j].r_offset;
          }
        }
      }
    }
  }
  return 0;
}

void so_finalize(void) {
  mprotect(text_virtbase, ALIGN_MEM(text_size, 0x1000), PROT_READ | PROT_EXEC);
}
