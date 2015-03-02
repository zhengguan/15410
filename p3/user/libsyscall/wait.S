/** @file wait.S
 *  @brief The wait system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include<syscall_int.h>

.globl wait

wait:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    mov     8(%ebp), %esi
    int     $WAIT_INT
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
