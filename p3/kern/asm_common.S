/** @file asm_common.S
 *  @brief Commonly used assembly functions.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <seg.h>

.globl jmp_ureg
jmp_ureg:
    mov     4(%esp), %esp
    addl    $8, %esp
    pop     %ds
    pop     %es
    pop     %fs
    pop     %gs
    popa
    addl    $4, %esp
    iret

.globl set_kernel_segs
set_kernel_segs:
    mov     $SEGSEL_KERNEL_DS, %eax
    mov     %eax, %ds
    mov     %eax, %es
    ret

.globl set_user_segs
set_user_segs:
    mov     $SEGSEL_USER_DS, %eax
    mov     %eax, %ds
    mov     %eax, %es
    ret
