/** @file swexn.S
 *  @brief The exn_handler_complete system-call stub.
 *
 *  A system call made by swexn before returning to the calling user program.
 *  Allows the kernel to pre
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <syscall_int.h>

.globl exn_handler_complete

exn_handler_complete:
    int $SYSCALL_RESERVED_0
    ret
