/** @file deschedule.S.
 *  @brief The deschedule system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include<syscall_int.h>

.globl deschedule

deschedule:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    mov     8(%ebp), %esi
    int     $DESCHEDULE_INT
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
