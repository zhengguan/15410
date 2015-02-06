/** @file misbehave.S.
 *  @brief The misbehave system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include<syscall_int.h>

.globl misbehave

misbehave:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    mov     8(%ebp), %esi
    int     $MISBEHAVE_INT
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
