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
    lprintf("readfile: %s", filename);
    int ret = getbytes(filename, offset, count, buf);
    if (ret < 0)
      lprintf("readfile failed with %d on %s with offset %d", ret, filename, offset);
    return ret;
}
