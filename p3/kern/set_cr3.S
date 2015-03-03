/** @file set_cr3.S
 *  @brief Sets the %cr3 control register.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

.globl set_cr3

set_cr3:
    push    %ebp
    mov     %esp, %ebp
    mov     8(%ebp), %cr3
    mov     %ebp, %esp
    pop     %ebp
    ret
