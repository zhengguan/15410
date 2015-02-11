/** @file swexn.S
 *  @brief The swexn system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include<syscall_int.h>

.globl swexn

swexn:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    lea     8(%ebp), %esi
    int     $SWEXN_INT
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
