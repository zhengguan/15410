/** @file readfile.c
 *  @brief This file implements the readfile system call.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <syscall.h>
#include <loader.h>

int readfile(char *filename, char *buf, int count, int offset)
{
    return getbytes(filename, offset, count, buf);
}
