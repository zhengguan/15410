/** @file vanish.S
 *  @brief The vanish system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include<syscall_int.h>

.globl vanish

vanish:
    push    %ebp
    mov     %esp, %ebp
    int     $VANISH_INT
    mov     %ebp, %esp
    pop     %ebp
    ret
