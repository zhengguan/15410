/** @file sizefile.S
 *  @brief The sizefile system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include<syscall_int.h>

.globl sizefile

sizefile:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    mov     8(%ebp), %esi
    int     $SIZEFILE_INT
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
