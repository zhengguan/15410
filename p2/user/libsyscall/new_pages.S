/** @file new_pages.S.
 *  @brief The new_pages system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include<syscall_int.h>

.globl new_pages

new_pages:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    lea     8(%ebp), %esi
    int     $NEW_PAGES_INT
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
