/** @file yield.S
 *  @brief The yield system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include<syscall_int.h>

.globl yield

yield:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    mov     8(%ebp), %esi
    int     $YIELD_INT
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
