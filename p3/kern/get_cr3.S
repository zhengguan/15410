/** @file get_cr3.S
 *  @brief Gets the %cr3 control register.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

.globl get_cr3

get_cr3:
    push    %ebp
    mov     %esp, %ebp
    mov     %cr3, %eax
    mov     %ebp, %esp
    pop     %ebp
    ret
