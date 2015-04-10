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
#include <asm_common.h>
#include <loader.h>
#include <kern_common.h>
#include <cr.h>
#include <vm.h>
#include <simics.h>
#include <hashtable.h>
#include <assert.h>
#include <exception.h>
#include <proc.h>

//MUST BE PAGE ALIGNED
#define USER_STACK_TOP ((char*)0xC0000000u)
#define USER_ARGV_START ((char*)USER_STACK_TOP)
#define USER_STACK_SIZE PAGE_SIZE

#define MAX_TOTAL_ARG_LEN 2048
#define MAX_ARG_NUM 512

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
    if (size < 0)
        return -3;
    for (i = 0; i < exec2obj_userapp_count; i++) {
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
static int alloc_pages(unsigned start, unsigned len, bool readonly)
{
    unsigned base;
    for (base = ROUND_DOWN_PAGE(start); base < start + len; base += PAGE_SIZE) {
        if (!vm_is_present((void *)base) &&
            (new_pages((void*)base, PAGE_SIZE) < 0)) {
            return -1;
        }

        if (readonly) {
            vm_read_only((void*)base);
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
                         unsigned arg_lens[], char tmp_args[])
{
    char *bottom_arg_ptr = USER_ARGV_START + sizeof(char*)*MAX_ARG_NUM;

    /* Copy arguments to argument space */
    if (new_pages(USER_ARGV_START, PAGE_SIZE) < 0) {
        proc_kill_thread("Killing thread. Out of memory.");
    }

    char **new_argv = (char**)(USER_ARGV_START);

    int i;
    for (i = 0; i < argc; i++) {
        memcpy(bottom_arg_ptr, tmp_args, arg_lens[i] + 1);
        new_argv[i] = bottom_arg_ptr;
        bottom_arg_ptr += arg_lens[i] + 1;
        tmp_args += arg_lens[i] + 1;
    }

    if (alloc_pages(se_hdr->e_txtstart, se_hdr->e_txtlen, true) < 0 ||
        alloc_pages(se_hdr->e_datstart, se_hdr->e_datlen, false) < 0 ||
        alloc_pages(se_hdr->e_rodatstart, se_hdr->e_rodatlen, true) < 0 ||
        alloc_pages(se_hdr->e_bssstart, se_hdr->e_bsslen, false) < 0) {
        proc_kill_thread("Killing thread. Out of memory.");
    }

    assert (getbytes(se_hdr->e_fname, se_hdr->e_txtoff, se_hdr->e_txtlen,
                (char*)se_hdr->e_txtstart) == se_hdr->e_txtlen &&
            getbytes(se_hdr->e_fname, se_hdr->e_datoff, se_hdr->e_datlen,
                (char*)se_hdr->e_datstart) == se_hdr->e_datlen &&
            getbytes(se_hdr->e_fname, se_hdr->e_rodatoff, se_hdr->e_rodatlen,
                (char*)se_hdr->e_rodatstart) == se_hdr->e_rodatlen);
    memset((char*)se_hdr->e_bssstart, 0, se_hdr->e_bsslen);

    char *stack_low = USER_STACK_TOP - USER_STACK_SIZE;

    if (new_pages(stack_low, USER_STACK_SIZE) < 0) {
        proc_kill_thread("Killing thread. Out of memory.");
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

    esp -= sizeof(int);

    return esp;
}


int str_arr_len(char *arr[])
{
    int len = 0;
    while (arr[len] != NULL)
        len++;

    return len;
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
int load(char *filename, char *argv[], unsigned *eip, unsigned *esp)
{
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

    int argc = str_arr_len(argv);

    if (argc > MAX_ARG_NUM)
        return -5;

    unsigned *arg_lens = NULL;
    if (argc > 0)
        if ( (arg_lens = malloc(argc*sizeof(unsigned))) == NULL)
            return -6;

    /* Get argument lengths */
    unsigned total_arg_len = 0;
    int i;
    for (i = 0; i < argc; i++) {
        arg_lens[i] = strlen(argv[i]);
        total_arg_len += arg_lens[i] + 1;
    }

    if (total_arg_len > MAX_EXECNAME_LEN) {
        free(arg_lens);
        return -8;
    }

    /* Copy arguments to temp kernel space */
    char *tmp_args = NULL;
    if (argc > 0)
        if ( (tmp_args = malloc(sizeof(char) * total_arg_len)) == NULL) {
            free(arg_lens);
            return -9;
        }

    char *arg_ptr = tmp_args;
    for (i = 0; i < argc; i++) {
        strncpy(arg_ptr, argv[i], arg_lens[i] + 1);
        arg_ptr += arg_lens[i] + 1;
    }

    vm_clear();
    *esp = fill_mem(&se_hdr, argc, argv, arg_lens, tmp_args);
    *eip = se_hdr.e_entry;
    free(arg_lens);
    free(tmp_args);

    deregister_swexn_handler(*CUR_PCB);

    if (*esp == 0) {
        return -9;
    }

    return 0;
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
    if ((unsigned)filename < USER_MEM_START ||
        (unsigned)argv < USER_MEM_START) {
        return -1;
    }

    int len;
    if ( (len = str_check(filename)) < 0)
        return -2;
    if (str_arr_check(argv) < 0)
        return -3;

    char *new_filename;
    if ( (new_filename = malloc((len+1) * sizeof(char))) == NULL)
        return -6;

    strcpy(new_filename, filename);

    unsigned eip, esp;
    if (load(new_filename, argv, &eip, &esp) < 0)
        return -4;
    free(new_filename);

    jmp_user(eip, esp);

    return -5;
}

/*@}*/
