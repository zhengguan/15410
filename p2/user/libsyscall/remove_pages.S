/** @file remove_pages.S
 *  @brief The remove_pages system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include<syscall_int.h>

.globl remove_pages

remove_pages:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    mov     8(%ebp), %esi
    int     $REMOVE_PAGES_INT
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
