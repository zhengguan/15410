/** @file task_vanish.S
 *  @brief The task_vanish system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include<syscall_int.h>

.globl task_vanish

task_vanish:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    mov     8(%ebp), %esi
    int     $TASK_VANISH_INT
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
