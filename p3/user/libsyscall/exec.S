/** @file exec.S
 *  @brief The exec system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include<syscall_int.h>

.globl exec

exec:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    lea     8(%ebp), %esi
    int     $EXEC_INT
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
