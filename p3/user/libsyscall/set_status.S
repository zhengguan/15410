/** @file set_status.S
 *  @brief The set_status system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include<syscall_int.h>

.globl set_status

set_status:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    mov     8(%ebp), %esi
    int     $SET_STATUS_INT
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
