/** @file halt.S.
 *  @brief The halt system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include<syscall_int.h>

.globl halt

halt:
    push    %ebp
    mov     %esp, %ebp
    int     $HALT_INT
    mov     %ebp, %esp
    pop     %ebp
    ret
