/** @file make_runnable.S
 *  @brief The make_runnable system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include<syscall_int.h>

.globl make_runnable

make_runnable:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    mov     8(%ebp), %esi
    int     $MAKE_RUNNABLE_INT
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
