/** @file set_cursor_pos.S
 *  @brief The set_cursor_pos system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include<syscall_int.h>

.globl set_cursor_pos

set_cursor_pos:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    lea     8(%ebp), %esi
    int     $SET_CURSOR_POS_INT
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
