/** @file asm_common.S
 *  @brief Commonly used assembly functions.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <seg.h>
#include <kern_common.h>

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

.globl jmp_ureg_user
jmp_ureg_user:
    call set_user_segs
    mov     4(%esp), %esp
    addl    $24, %esp
    mov     %esp, %eax //fix dummy esp
    addl    $32, %eax
    mov     %eax, 12(%esp)
    popa
    addl    $4, %esp
    movl $SEGSEL_USER_CS, 4(%esp)
    movl $USER_EFLAGS, 8(%esp)
    movl $SEGSEL_USER_DS, 16(%esp)
    iret

.globl jmp_user
jmp_user:
    mov %esp, %eax
    push $SEGSEL_USER_DS
    push 8(%eax)
    push $USER_EFLAGS
    push $SEGSEL_USER_CS
    push 4(%eax)
.globl iret_user
iret_user:
    call set_user_segs
    mov $0, %ebp
    iret

.globl set_kernel_segs
set_kernel_segs:
    mov     $SEGSEL_KERNEL_DS, %eax
    mov     %eax, %ds
    mov     %eax, %es
    mov     %eax, %fs
    mov     %eax, %gs
    ret

.globl set_user_segs
set_user_segs:
    mov     $SEGSEL_USER_DS, %eax
    mov     %eax, %ds
    mov     %eax, %es
    mov     %eax, %fs
    mov     %eax, %gs
    ret

.globl flush_tlb
flush_tlb:
    mov %cr3, %eax
    mov %eax, %cr3
    ret

.globl flush_tlb_entry
flush_tlb_entry:
    mov     4(%esp), %eax
    invlpg  (%eax)
    ret