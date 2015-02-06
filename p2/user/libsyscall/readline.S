/** @file readline.S.
 *  @brief The readline system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include<syscall_int.h>

.globl set_term_color

set_term_color:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    mov     8(%ebp), %esi
    int     $READLINE_INT
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
