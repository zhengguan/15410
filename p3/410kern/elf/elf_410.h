/* The authorship of the 410 simplified ELF structure seems to be lost to time
 * immemorial.  This file has been rescued and broken out to stand on its own.
 */

#ifndef _ELF_ELF_410_H_
#define _ELF_ELF_410_H_

#include <elf/elf32.h>

/* --- Simplified ELF header --- */
typedef struct simple_elf {
  const char *  e_fname;       /* filename of binary */
  unsigned long e_entry;       /* entry point virtual address */
  unsigned long e_txtoff;      /* offset of text segment in file */
  unsigned long e_txtlen;      /* length of text segment in bytes */
  unsigned long e_txtstart;    /* start of text segment virtual address */
  unsigned long e_datoff;      /* offset of data segment in file */
  unsigned long e_datlen;      /* length of data segment in bytes */
  unsigned long e_datstart;    /* start of data segment in virtual memory */
  unsigned long e_rodatoff;    /* offset of rodata segment in file */
  unsigned long e_rodatlen;    /* length of rodata segment in bytes */
  unsigned long e_rodatstart;  /* start of rodata segment in virtual memory*/
  unsigned long e_bsslen;      /* length of bss  segment in bytes */
  unsigned long e_bssstart;    /* start of bss  segment in virtual memory */
} simple_elf_t;

 /* --- Defines --- */
#define ELF_NOTELF  -1
#define ELF_SUCCESS  0
#define NOT_PRESENT  -2

/*
 * elf function prototypes
 */
extern int getbytes( const char *filename, int offset, int size, char *buf);
extern int elf_load_helper( simple_elf_t *se_hdr, const char *fname );
extern int elf_check_header( const char *fname );

#endif
