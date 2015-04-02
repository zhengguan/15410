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
#include <syscall.h>
#include <common_kern.h>
#include <loader.h>
#include <exec_run.h>
#include <kern_common.h>
#include <cr.h>
#include <vm.h>
#include <simics.h>
#include <hashtable.h>

// TODO change this (top of stack?)

//MUST BE PAGE ALIGNED
#define USER_ARGV_TOP ((char*)0xD0000000u)
#define USER_STACK_TOP ((char*)0xC0000000u)
#define USER_STACK_SIZE PAGE_SIZE

#define USER_MODE_CPL 3

#define PAGE_HT_SIZE 10

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
            if (offset+size > entry->execlen) {
                return -1;
            }
            int len = MIN(entry->execlen - offset, size);
            memcpy(buf, entry->execbytes + offset, len);
            return len;
        }
    }

    return -2;
}

/**
 * @brief Performs simple checks to determine if a ELF file is valid.
 *
 * @param se_hdr simple_elf_t
 * @return True if the elf header is valid, false otherwise.
 */
static bool elf_valid(const simple_elf_t *se_hdr)
{
    if (se_hdr->e_entry < se_hdr->e_txtstart ||
        se_hdr->e_entry >= se_hdr->e_txtstart + se_hdr->e_txtlen) {
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

/** @brief Allocate memory one page at a time if the page is not already
 *  allocated
 *
 *  @param start The start of the memory region to allocate.
 *  @param len The length of the memory region to allocate.
 *  @return 0 on success, negative error code otherwise.
 */
static int alloc_pages(unsigned start, unsigned len)
{
    unsigned base;
    for (base = ROUND_DOWN_PAGE(start); base < start + len; base += PAGE_SIZE) {
        if (!vm_is_present((void *)base) &&
            (new_pages((void*)base, PAGE_SIZE) < 0)) {
            return -1;
        }
    }
    return 0;
}

/** @brief Fill memory regions needed to run a program.
 *
 *  Does not modify user memory on failure.
 *
 *  @param se_hdr The ELF header.
 *  @param argc The number strings in argv.
 *  @param argv The function arguments.
 *  @param arg_lens The length of each string in argv.
 *  @return The inital value of the stack pointer for the function, 0 on
 *  failure.
 */
static unsigned fill_mem(const simple_elf_t *se_hdr, int argc, char *argv[],
                         int arg_lens[])
{
    //TODO limit arg length

    char *bottom_arg_ptr = USER_ARGV_TOP;

    int total_arg_len = 0;
    int i;
    for (i = 0; i < argc; i++) {
        total_arg_len += arg_lens[i] + 1;
    }

    int arg_mem_needed = ROUND_UP_PAGE(sizeof(char) * total_arg_len +
                                       sizeof(char*) * argc);

    //TODO: better check
    if (arg_mem_needed > USER_ARGV_TOP - USER_STACK_TOP) {
        return 0;
    }

    char *txt_buf = malloc(se_hdr->e_txtlen + se_hdr->e_datlen +
                           se_hdr->e_rodatlen);
    if (txt_buf == NULL) {
        return 0;
    }
    char *dat_buf = txt_buf + se_hdr->e_txtlen;
    char *rodat_buf = dat_buf + se_hdr->e_datlen;

    if (getbytes(se_hdr->e_fname, se_hdr->e_txtoff,
                 se_hdr->e_txtlen, txt_buf) < 0 ||
        getbytes(se_hdr->e_fname, se_hdr->e_datoff,
                 se_hdr->e_datlen, dat_buf) < 0 ||
        getbytes(se_hdr->e_fname, se_hdr->e_rodatoff,
                 se_hdr->e_rodatlen, rodat_buf) < 0) {
        free(txt_buf);
        return 0;
    }

    char *tmp_args = malloc(sizeof(char) * total_arg_len);
    if (tmp_args == NULL) {
        free(txt_buf);
        return 0;
    }

    for (i = 0; i < argc; i++) {
        strncpy(tmp_args, argv[i], arg_lens[i] + 1);
        tmp_args += arg_lens[i] + 1;
    }

    vm_clear();

    if (arg_mem_needed > 0 && (new_pages(bottom_arg_ptr -
        arg_mem_needed, arg_mem_needed) < 0) ) {
        // TODO handle this
        lprintf("oops");
        MAGIC_BREAK;
    }

    char **new_argv = (char**)(bottom_arg_ptr - arg_mem_needed);
    for (i = argc - 1; i >= 0; i--) {
        bottom_arg_ptr -= arg_lens[i] + 1;
        tmp_args -= arg_lens[i] + 1;
        strncpy(bottom_arg_ptr, tmp_args, arg_lens[i] + 1);
        new_argv[i] = bottom_arg_ptr;
    }

    free(tmp_args);

    if (alloc_pages(se_hdr->e_txtstart, se_hdr->e_txtlen) < 0 ||
        alloc_pages(se_hdr->e_datstart, se_hdr->e_datlen) < 0 ||
        alloc_pages(se_hdr->e_rodatstart, se_hdr->e_rodatlen) < 0 ||
        alloc_pages(se_hdr->e_bssstart, se_hdr->e_bsslen) < 0) {
        // TODO handle this
        MAGIC_BREAK;
    }

    // TODO make txt and rodata read only?
    memcpy((char*)se_hdr->e_txtstart, txt_buf, se_hdr->e_txtlen);
    memcpy((char*)se_hdr->e_datstart, dat_buf, se_hdr->e_datlen);
    memcpy((char*)se_hdr->e_rodatstart, rodat_buf, se_hdr->e_rodatlen);
    memset((char*)se_hdr->e_bssstart, 0, se_hdr->e_bsslen);

    free(txt_buf);

    char *stack_low = USER_STACK_TOP - USER_STACK_SIZE;

    if (new_pages(stack_low, USER_STACK_SIZE) < 0) {
        // TODO handle this
        MAGIC_BREAK;
    }

    unsigned esp = (unsigned)USER_STACK_TOP;

    esp -= sizeof(int);
    *(unsigned*)esp = (unsigned)stack_low;

    esp -= sizeof(int);
    *(unsigned*)esp = (unsigned)(USER_STACK_TOP - 1);

    esp -= sizeof(char**);
    *(char ***)esp = new_argv;

    esp -= sizeof(int);
    *(int*)esp = argc;

    //TODO: need make a wrapper so we don't crash if they DO return?
    //push return address (isn't one)
    esp -= sizeof(int);

    return esp;
}

/** @brief Begins running a user program.
 *
 *  @param eip The initial value of the instruction pointer.
 *  @param esp The inital value of the stack pointer.
 *  @return Does not return.
 */
static void user_run(unsigned eip, unsigned esp)
{
    //TODO: should this look at currently set eflags?
    //TODO: more flags?
    set_cr0((get_cr0() & ~CR0_AM & ~CR0_WP) | CR0_PE);
    unsigned eflags = EFL_RESV1 | EFL_IF | EFL_IOPL_RING1;
    exec_run(SEGSEL_USER_DS, SEGSEL_USER_DS, eip, SEGSEL_USER_CS,
             eflags, esp, SEGSEL_USER_DS);
}

/** @brief Loads a user program.
 *
 *  Replaces the program currently running in the invoking task with
 *  the program stored in the file named execname.  The argument points to a
 *  null-terminated vector of null-terminated string arguments.
 *
 *  @param filename The program file name.
 *  @param argv The argument vector.
 *  @param kernel_mode A boolean indicating whether it is legal for arguments
 *  to point to kernel memory.
 *  @return Does not return on success, a negative error code on
 *  failure.
 */
int load(char *filename, char *argv[], bool kernel_mode)
{
    // TODO what is the purpose of kernel_mode?
    if (!kernel_mode && ((unsigned)filename < USER_MEM_START ||
        (unsigned)argv < USER_MEM_START)) {
        return -1;
    }

    int len = str_check(filename);
    if (len < 0) {
        return -1;
    }

    if (elf_check_header(filename) != ELF_SUCCESS) {
        return -2;
    }

    simple_elf_t se_hdr;
    if (elf_load_helper(&se_hdr, filename) != ELF_SUCCESS) {
        return -3;
    }

    if (!elf_valid(&se_hdr)) {
        return -4;
    }

    int argc = null_arr_check(argv);
    if (argc < 0) {
        return -5;
    }

    int *arg_lens  = malloc(argc*sizeof(int));
    if (argc > 0 && arg_lens == NULL) {
        return -7;
    }

    //get lengths of arguments and ensure they are valid strings
    int i;
    for (i = 0; i < argc; i++) {
        int len;
        if ( (!kernel_mode && (unsigned)argv[i] < USER_MEM_START) ||
             (len = str_check(argv[i])) < 0) {
            free(arg_lens);
            return -8;
        }

        arg_lens[i] = len;
    }

    unsigned esp = fill_mem(&se_hdr, argc, argv, arg_lens);

    free(arg_lens);

    if (esp == 0) {
        return -9;
    }

    user_run(se_hdr.e_entry, esp);

    //should never get here

    return -10;
}

/** @brief Loads a user program.
 *
 *  A wrapper for load with kernel_mode set to false.
 *
 *  @param filename The program file name.
 *  @param argv The argument vector.
 *  @return Does not return on success, a negative error code on
 *  failure.
 */
int exec(char *filename, char *argv[])
{
    return load(filename, argv, false);
}

/*@}*/
