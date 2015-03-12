/**
 * The 15-410 kernel project.
 * @name loader.c
 *
 * Functions for the loading
 * of user programs from binary
 * files should be written in
 * this file. The function
 * elf_load_helper() is provided
 * for your use.
 */
/*@{*/

/* --- Includes --- */
#include <string.h>
#include <stdio.h>
#include <malloc.h>
#include <exec2obj.h>
#include <elf_410.h>
#include <eflags.h>
#include <seg.h>
#include <common_kern.h>
#include <loader.h>
#include <exec_run.h>
#include <macros.h>

// TODO change this (top of stack?)
#define USER_STACK_TOP 0xBFFFFFFF
#define USER_STACK_SIZE PAGE_SIZE 
#define USER_MODE_CPL 3


/* --- Local function prototypes --- */


/**
 * Copies data from a file into a buffer.
 *
 * @param filename   the name of the file to copy data from
 * @param offset     the location in the file to begin copying from
 * @param size       the number of bytes to be copied
 * @param buf        the buffer to copy the data into
 *
 * @return returns the number of bytes copied on success; -1 on failure
 */
int getbytes(const char *filename, int offset, int size, char *buf)
{
    int i;
    for (i = 0; i < MAX_NUM_APP_ENTRIES; i++) {
        const exec2obj_userapp_TOC_entry *entry = &exec2obj_userapp_TOC[i];
        if (!strncmp(entry->execname, filename, MAX_EXECNAME_LEN)) {
            if (offset >= entry->execlen) {
                return -1;
            }
            int len = MIN(entry->execlen - offset, size);
            memcpy(buf, entry->execbytes + offset, len);
            return len;
        }
    }

    return -1;
}

static bool elf_valid(const simple_elf_t *se_hdr)
{
    if (se_hdr->e_entry < se_hdr->e_txtstart || se_hdr->e_entry >= se_hdr->e_txtstart + se_hdr->e_txtlen) {
        return false;
    }
    
    if (se_hdr->e_txtstart < USER_MEM_START) {
        return false;
    }
    
    if (se_hdr->e_datstart < USER_MEM_START && se_hdr->e_datlen != 0) {
        return false;
    }
    
    if (se_hdr->e_rodatstart < USER_MEM_START && se_hdr->e_rodatlen != 0) {
        return false;
    }
    
    if (se_hdr->e_bssstart < USER_MEM_START && se_hdr->e_bsslen != 0) {
        return false;
    }
    
    return true;
}

/** @brief Allocate page-aligned memory one page at a time.
 *
 *  @param start The start of the memory region to allocate.
 *  @param su The length of the memory region to allocate.
 *  @return Void.
 */
static void alloc_pages(unsigned start, unsigned len)
{
    unsigned base;
    for (base = ROUND_DOWN_PAGE(start); base < start + len; base += PAGE_SIZE) {
        new_pages((void*)base, PAGE_SIZE);
    }
}

/** @brief Fill memory regions needed to run a program.
 *
 *  @param se_efl The ELF header.
 *  @param argv The function arguments.
 *  @return The inital value of the stack pointer for the function.
 */
static unsigned fill_mem(const simple_elf_t *se_hdr, char *argv[])
{
    alloc_pages(se_hdr->e_txtstart, se_hdr->e_txtlen);
    alloc_pages(se_hdr->e_datstart, se_hdr->e_datlen);
    alloc_pages(se_hdr->e_rodatstart, se_hdr->e_rodatlen);
    alloc_pages(se_hdr->e_bssstart, se_hdr->e_bsslen);

    // TODO make txt and rodata read only
    if (getbytes(se_hdr->e_fname, se_hdr->e_txtoff, se_hdr->e_txtlen,
        (char *)se_hdr->e_txtstart) < 0) {
        return 0;
    }
    if (getbytes(se_hdr->e_fname, se_hdr->e_datoff, se_hdr->e_datlen,
        (char *)se_hdr->e_datstart) < 0) {
        return 0;
    }
    if (getbytes(se_hdr->e_fname, se_hdr->e_rodatoff, se_hdr->e_rodatlen,
        (char *)se_hdr->e_rodatstart) < 0) {
        return 0;
    }
    memset((char*)se_hdr->e_bssstart, 0, se_hdr->e_bsslen);

    int arglen;
    for (arglen = 0; argv[arglen] != NULL; arglen++);

    unsigned stack_low = USER_STACK_TOP - USER_STACK_SIZE + 1;
    new_pages((void*)(stack_low), USER_STACK_SIZE);

    unsigned esp = USER_STACK_TOP + 1;
    esp -= sizeof(int);
    *(int *)esp = stack_low;
    esp -= sizeof(int);
    *(int *)esp = USER_STACK_TOP;
    esp -= sizeof(char**);
    *(char ***)esp = argv;
    esp -= sizeof(int);
    *(int*)esp = arglen;
    esp -= sizeof(int);

    return esp;
}

/** @brief Begins running a user program.
 *
 *  @param eip The initial value of the instruction pointer.
 *  @param esp The inital value of the stack pointer.
 *  @return Does not return.
 */
void user_run(unsigned eip, unsigned esp)
{
    //TODO: should this look at currently set eflags?
    //TODO: more flags?
    unsigned eflags = EFL_RESV1 | EFL_IF;
    exec_run(SEGSEL_USER_DS, eip, SEGSEL_USER_CS, eflags, esp, SEGSEL_USER_DS);
}

int exec(char *filename, char *argv[])
{
    if (elf_check_header(filename) != ELF_SUCCESS) {
        return -1;
    }
    
    simple_elf_t se_hdr;
    if (elf_load_helper(&se_hdr, filename) != ELF_SUCCESS) {
        return -2;
    }
    
    if (!elf_valid(&se_hdr)) {
        return -3;
    }
    
    unsigned esp = fill_mem(&se_hdr, argv);
    if (esp == 0) {
        return -4;
    }
    
    user_run(se_hdr.e_entry, esp);

    return -5;
}

/*@}*/
