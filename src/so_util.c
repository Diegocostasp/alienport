/* so_util.c -- utils to load and hook .so modules
 *
 * Based on work by fgsfds, Andy Nguyen, and Jaakko Lukkari.
 */

#include <assert.h>
#include <elf.h>
#include <errno.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#include "so_util.h"

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

static inline size_t round_up(size_t x, size_t a) {
  return (x + a - 1) & ~(a - 1);
}

static int protect_range(void *start, size_t len, int prot) {
  int ps = getpagesize();
  if (ps <= 0) ps = 4096;
  uintptr_t addr = (uintptr_t)start;
  uintptr_t page_base = addr & ~((uintptr_t)ps - 1);
  size_t head = addr - page_base;
  size_t plen = round_up(len + head, (size_t)ps);
  if (mprotect((void *)page_base, plen, prot) != 0) {
    return -1;
  }
  return 0;
}

void so_flush_caches(void) {
  __builtin___clear_cache((char *)load_virtbase, (char *)load_virtbase + load_size);
}

void so_make_text_writable(void) {
  const size_t text_asize = ALIGN_MEM(text_size, 0x1000);
  mprotect(text_virtbase, text_asize, PROT_READ | PROT_WRITE | PROT_EXEC);
}

void so_make_text_executable(void) {
  const size_t text_asize = ALIGN_MEM(text_size, 0x1000);
  mprotect(text_virtbase, text_asize, PROT_READ | PROT_EXEC);
}

void so_finalize(void) {
  protect_range(text_virtbase, text_size, PROT_READ | PROT_EXEC);
  protect_range(data_virtbase, data_size, PROT_READ | PROT_WRITE);
}

int so_load(const char *filename, void *base, size_t max_size) {
  FILE *fd = fopen(filename, "rb");
  if (fd == NULL) return -1;

  fseek(fd, 0, SEEK_END);
  size_t so_size = ftell(fd);
  fseek(fd, 0, SEEK_SET);

  so_base = malloc(so_size);
  fread(so_base, so_size, 1, fd);
  fclose(fd);

  if (memcmp(so_base, ELFMAG, SELFMAG) != 0) return -1;

  elf_hdr = (Elf64_Ehdr *)so_base;
  if (elf_hdr->e_ident[EI_CLASS] != ELFCLASS64 || elf_hdr->e_machine != EM_AARCH64) return -1;

  prog_hdr = (Elf64_Phdr *)((uintptr_t)so_base + elf_hdr->e_phoff);
  sec_hdr = (Elf64_Shdr *)((uintptr_t)so_base + elf_hdr->e_shoff);
  shstrtab = (char *)((uintptr_t)so_base + sec_hdr[elf_hdr->e_shstrndx].sh_offset);

  int text_segno = -1, data_segno = -1;
  for (int i = 0; i < elf_hdr->e_phnum; i++) {
    if (prog_hdr[i].p_type == PT_LOAD) {
      const size_t prog_size = ALIGN_MEM(prog_hdr[i].p_memsz, prog_hdr[i].p_align);
      if ((prog_hdr[i].p_flags & PF_X) == PF_X) {
        text_segno = i;
      } else {
        data_segno = i;
        load_size = prog_hdr[i].p_vaddr + prog_size;
      }
    }
  }

  load_size = ALIGN_MEM(load_size, 0x1000);
  load_base = base;
  load_virtbase = load_base;
  memset(load_base, 0, load_size);

  text_size = prog_hdr[text_segno].p_memsz;
  text_virtbase = (void *)(prog_hdr[text_segno].p_vaddr + (Elf64_Addr)load_virtbase);
  text_base = (void *)(prog_hdr[text_segno].p_vaddr + (Elf64_Addr)load_base);
  memcpy(text_base, (void *)((uintptr_t)so_base + prog_hdr[text_segno].p_offset), prog_hdr[text_segno].p_filesz);

  data_size = prog_hdr[data_segno].p_memsz;
  data_virtbase = (void *)(prog_hdr[data_segno].p_vaddr + (Elf64_Addr)load_virtbase);
  data_base = (void *)(prog_hdr[data_segno].p_vaddr + (Elf64_Addr)load_base);
  memcpy(data_base, (void *)((uintptr_t)so_base + prog_hdr[data_segno].p_offset), prog_hdr[data_segno].p_filesz);

  syms = NULL;
  dynstrtab = NULL;
  for (int i = 0; i < elf_hdr->e_shnum; i++) {
    char *sh_name = shstrtab + sec_hdr[i].sh_name;
    if (strcmp(sh_name, ".dynsym") == 0) {
      syms = (Elf64_Sym *)((uintptr_t)text_base + sec_hdr[i].sh_addr);
      num_syms = sec_hdr[i].sh_size / sizeof(Elf64_Sym);
    } else if (strcmp(sh_name, ".dynstr") == 0) {
      dynstrtab = (char *)((uintptr_t)text_base + sec_hdr[i].sh_addr);
    }
  }

  return 0;
}

int so_relocate(void) {
  for (int i = 0; i < elf_hdr->e_shnum; i++) {
    char *sh_name = shstrtab + sec_hdr[i].sh_name;
    if (strcmp(sh_name, ".rela.dyn") == 0 || strcmp(sh_name, ".rela.plt") == 0) {
      Elf64_Rela *rels = (Elf64_Rela *)((uintptr_t)text_base + sec_hdr[i].sh_addr);
      for (int j = 0; j < sec_hdr[i].sh_size / sizeof(Elf64_Rela); j++) {
        uintptr_t *ptr = (uintptr_t *)((uintptr_t)text_base + rels[j].r_offset);
        Elf64_Sym *sym = &syms[ELF64_R_SYM(rels[j].r_info)];
        int type = ELF64_R_TYPE(rels[j].r_info);
        switch (type) {
          case R_AARCH64_ABS64: *ptr = (uintptr_t)text_virtbase + sym->st_value + rels[j].r_addend; break;
          case R_AARCH64_RELATIVE: *ptr = (uintptr_t)text_virtbase + rels[j].r_addend; break;
          case R_AARCH64_GLOB_DAT:
          case R_AARCH64_JUMP_SLOT:
            if (sym->st_shndx != SHN_UNDEF) *ptr = (uintptr_t)text_virtbase + sym->st_value + rels[j].r_addend;
            break;
        }
      }
    }
  }
  return 0;
}

int so_resolve(DynLibFunction *funcs, int num_funcs, int taint_missing_imports) {
  for (int i = 0; i < elf_hdr->e_shnum; i++) {
    char *sh_name = shstrtab + sec_hdr[i].sh_name;
    if (strcmp(sh_name, ".rela.dyn") == 0 || strcmp(sh_name, ".rela.plt") == 0) {
      Elf64_Rela *rels = (Elf64_Rela *)((uintptr_t)text_base + sec_hdr[i].sh_addr);
      for (int j = 0; j < sec_hdr[i].sh_size / sizeof(Elf64_Rela); j++) {
        uintptr_t *ptr = (uintptr_t *)((uintptr_t)text_base + rels[j].r_offset);
        Elf64_Sym *sym = &syms[ELF64_R_SYM(rels[j].r_info)];
        if (ELF64_R_TYPE(rels[j].r_info) == R_AARCH64_GLOB_DAT || ELF64_R_TYPE(rels[j].r_info) == R_AARCH64_JUMP_SLOT) {
          if (sym->st_shndx == SHN_UNDEF) {
            char *name = dynstrtab + sym->st_name;
            for (int k = 0; k < num_funcs; k++) {
              if (strcmp(name, funcs[k].symbol) == 0) {
                *ptr = funcs[k].func;
                break;
              }
            }
          }
        }
      }
    }
  }
  return 0;
}

void so_execute_init_array(void) {
  for (int i = 0; i < elf_hdr->e_shnum; i++) {
    if (strcmp(shstrtab + sec_hdr[i].sh_name, ".init_array") == 0) {
      int (**init_array)() = (void *)((uintptr_t)text_virtbase + sec_hdr[i].sh_addr);
      for (int j = 0; j < sec_hdr[i].sh_size / 8; j++) {
        if (init_array[j]) init_array[j]();
      }
    }
  }
}

uintptr_t so_find_addr(const char *symbol) {
  for (int i = 0; i < num_syms; i++) {
    if (strcmp(dynstrtab + syms[i].st_name, symbol) == 0) return (uintptr_t)text_base + syms[i].st_value;
  }
  return 0;
}

uintptr_t so_find_addr_rx(const char *symbol) {
  for (int i = 0; i < num_syms; i++) {
    if (strcmp(dynstrtab + syms[i].st_name, symbol) == 0) return (uintptr_t)text_virtbase + syms[i].st_value;
  }
  return 0;
}
