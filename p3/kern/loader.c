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
#include <loader.h>
#include <elf_410.h>
#include <macros.h>
#include <common_kern.h>
#include <simics.h>
#include <exec_run.h>

#define USER_STACK_TOP ((unsigned)0xFFFFFFFF)


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
        const exec2obj_userapp_TOC_entry *entry = exec2obj_userapp_TOC+i;
        if (!strncmp(entry->execname, filename, MAX_EXECNAME_LEN)) {
            if (offset >= entry->execlen)
                return -1;
            int min = MIN(entry->execlen-offset, size);
            memcpy(buf, entry->execbytes+offset, min);
            return min;
        }
    }

    return -1;
}

int elf_valid(const simple_elf_t *se_hdr, const exec2obj_userapp_TOC_entry *entry)
{
    //TODO: add more checks
    if (se_hdr->e_entry < se_hdr->e_txtstart || se_hdr->e_entry >= se_hdr->e_txtstart+se_hdr->e_txtlen)
        return 0;
    if (se_hdr->e_txtstart < USER_MEM_START)
        return 0;
    if (se_hdr->e_datstart < USER_MEM_START && se_hdr->e_datlen != 0)
        return 0;
    if (se_hdr->e_rodatstart < USER_MEM_START && se_hdr->e_rodatlen != 0)
        return 0;
    if (se_hdr->e_bssstart < USER_MEM_START && se_hdr->e_bsslen != 0)
        return 0;

    if (se_hdr->e_txtoff >= entry->execlen)
        return 0;
    if (se_hdr->e_datoff >= entry->execlen)
        return 0;
    if (se_hdr->e_rodatoff >= entry->execlen)
        return 0;

    return 1;

}

void allocate_pages(unsigned start, unsigned len)
{
    unsigned end = start + len;

    unsigned cur = ROUND_DOWN_TO_PAGE(start);

    while (cur < end) {
        new_pages((void*)cur, PAGE_SIZE);
        cur += PAGE_SIZE;
    }
}

unsigned fillmem(const simple_elf_t *se_hdr, const exec2obj_userapp_TOC_entry *entry, char *argv[])
{
    //TODO: who you gonna call? new pages!

    //TODO: errors
    //TODO: page align
    allocate_pages(se_hdr->e_txtstart, se_hdr->e_txtlen);
    allocate_pages(se_hdr->e_datstart, se_hdr->e_datlen);
    allocate_pages(se_hdr->e_rodatstart, se_hdr->e_rodatlen);
    allocate_pages(se_hdr->e_bssstart, se_hdr->e_bsslen);

    memcpy((char*)se_hdr->e_txtstart, entry->execbytes + se_hdr->e_txtoff, se_hdr->e_txtlen);

    memcpy((char*)se_hdr->e_datstart, entry->execbytes + se_hdr->e_datoff, se_hdr->e_datlen);
    //TODO: make this read only?
    memcpy((char*)se_hdr->e_rodatstart, entry->execbytes + se_hdr->e_rodatoff, se_hdr->e_rodatlen);

    memset((char*)se_hdr->e_bssstart, 0, se_hdr->e_bsslen);

    lprintf("1");
    int arglen;
    for (arglen = 0; argv[arglen] != NULL; arglen++);
    lprintf("2");

    unsigned stack_low = USER_STACK_TOP-PAGE_SIZE+1;
    new_pages((void*)(stack_low), PAGE_SIZE);

    unsigned esp = USER_STACK_TOP+1;
    esp -= sizeof(int);
    *(int*)esp = stack_low;
    esp -= sizeof(int);
    *(int*)esp = USER_STACK_TOP;
    esp -= arglen*sizeof(char);
    memcpy((char*)esp, argv, arglen*sizeof(char*));
    esp -= sizeof(int);
    *(int*)esp = arglen;

    return esp;
}

int exec(char *filename, char *argv[])
{
    const exec2obj_userapp_TOC_entry *entry;

    int i;
    for (i = 0; i < MAX_NUM_APP_ENTRIES; i++) {
        entry = exec2obj_userapp_TOC+i;
        if (!strncmp(entry->execname, filename, MAX_EXECNAME_LEN)) {
            break;
        }
    }

    if (i == MAX_NUM_APP_ENTRIES)
        return -1;

    simple_elf_t se_hdr;

    if (elf_load_helper(&se_hdr, filename) == ELF_SUCCESS) {
        if (!elf_valid(&se_hdr, entry)) {
            return -2;
        }
        lprintf("filling mem");

        /* start new elf */
        unsigned esp = fillmem(&se_hdr, entry, argv);

        lprintf("filled");

        exec_run(esp, se_hdr.e_entry);

        //should not get here
        lprintf("fucked up");
        MAGIC_BREAK;
    }
    lprintf("here");
    return -1;
}

/*@}*/
