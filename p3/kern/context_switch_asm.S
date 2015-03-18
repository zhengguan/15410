/** @file context_switch_asm.S
 *  @brief Implementation of context_switch_asm
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

//TODO: take stack segment as arg

.globl context_switch_asm
context_switch_asm:
    push %ebp
    mov %esp, %ebp
    push $RETURN
    pusha
    pushf
    call get_cr0
    push %eax
    call get_cr2
    push %eax
    call get_cr3
    push %eax
    call get_cr4
    push %eax
    //push %cs
    push %ds
    push %es
    push %fs
    push %gs
    mov 4(%ebp), %esp
    pop %gs
    pop %fs
    pop %es
    pop %ds
    //pop %cs
    call set_cr4
    addl 4, %esp
    call set_cr3
    addl 4, %esp
    call set_cr2
    addl 4, %esp
    call set_cr0
    addl 4, %esp
    popf
    popa
    ret
  RETURN:
    leave
    ret