/** @file fork.S
 *  @brief The fork system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include<syscall_int.h>

.globl fork

fork:
    push    %ebp
    mov     %esp, %ebp
    int     $FORK_INT
    mov     %ebp, %esp
    pop     %ebp
    ret
