/** @file readfile.c
 *  @brief This file implements the readfile system call.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <syscall.h>
#include <stdlib.h>
#include <loader.h>
#include <vm.h>
#include <simics.h>

int readfile(char *filename, char *buf, int count, int offset)
{
    if (str_check(filename) < 0)
        return -4;
    //FIXME: what if they deallocate buf after we check?
    if ((unsigned)buf < USER_MEM_START || !vm_is_present_len(buf, count))
        return -5;

    return getbytes(filename, offset, count, buf);
}