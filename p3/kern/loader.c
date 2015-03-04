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
#include <spec/common_kern.h>

#define USER_STACK_TOP ((char*)0xFFFFFFFF)


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
int getbytes( const char *filename, int offset, int size, char *buf )
{
    int i;
    for (i = 0; i < MAX_NUM_APP_ENTRIES; i++) {
        exec2obj_userapp_TOC_entry *entry = exec2obj_userapp_TOC+i;
        if (!strncmp(entry->execname, filename, MAX_EXECNAME_LEN)) {
            if (offset >= entry->execlen)
                return -1;
            memcpy(buf, entry->execbytes+offset, min(entry->execlen-offset, size));
        }
    }

    return -1;
}

int elf_valid(const simple_elf_t *se_hdr, const exec2obj_userapp_TOC_entry *entry)
{
    //TODO: add more checks
    if (se_hdr->e_entry < se_hdr->e_txtstart || se_hdr->e_entry >= se_hdr->e_txtstart+e_txtlen)
        return 0;
    if (se_hdr->e_txtstart < USER_MEM_START)
        return 0;
    if (se_hdr->e_datstart < USER_MEM_START)
        return 0;
    if (se_hdr->e_rodatstart < USER_MEM_START)
        return 0;
    if (se_hdr->e_bssstart < USER_MEM_START)
        return 0;

    if (se_hdr->e_txtoff >= entry->execlen)
        return 0;
    if (se_hdr->e_datoff >= entry->execlen)
        return 0;
    if (se_hdr->e_rodatoff >= entry->execlen)
        return 0;
    if (se_hdr->e_bssoff >= entry->execlen)
        return 0;

    return 1;

}

int fillmem(const simple_elf_t *se_hdr, const exec2obj_userapp_TOC_entry *entry, const char *argv[])
{
    //TODO: who you gonna call? new pages!

    memcpy(se_hdr->e_txtstart, entry->execbytes + se_hdr->e_txtoff, se_hdr->e_txtlen);
    memcpy(se_hdr->e_datstart, entry->execbytes + se_hdr->e_datoff, se_hdr->e_datlen);
    //TODO: make this read only?
    memcpy(se_hdr->e_rodatstart, entry->execbytes + se_hdr->e_rodatoff, se_hdr->e_rodatlen);
    memset(se_hdr->e_bssstart, 0, se_hdr->e_bsslen);

    int arglen;
    for (arglen = 0; argv[arglen] != NULL; arglen++);

    char *esp = USER_STACK_TOP - (sizeof(char*) * arglen) - sizeof(int) + 1;

    memcpy(esp + sizeof(int), argv, arglen*sizeof(char*));
    *(int*)esp = arglen;

    return esp;
}

int exec(const char *filename, const char *argv[])
{
    if (!elf_valid(se_hdr, entry))
        return -1;

    exec2obj_userapp_TOC_entry *entry;

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

        /* start new elf */
        unsigned esp = fillmem(&se_hdr, entry, argv);

        exec_run(esp, se_hdr.e_entry);

        //should not get here
        lprintf("fucked up");
        MAGIC_BREAK;
    }
    return -3;
}

/*@}*/
