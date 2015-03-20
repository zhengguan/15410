/** @file context_switch_asm.S
 *  @brief Implementation of context_switch_asm
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

//TODO: take stack segment as arg



.globl context_switch_asm
.globl store_registers_asm


//prob dont need to save caller save registers or eip

store_regs:
    push    %ebp
    mov     %esp, %ebp     
    mov     8(%ebp), %eax       # use %eax because it is caller-saved
    mov     $0, (%eax)
    mov     %ebx, 4(%eax)
    mov     %ecx, 8(%eax)
    mov     %edx, 12(%eax)
    mov     %esi, 16(%eax)
    mov     %edi, 20(%eax)
    mov     %esp, 24(%eax)
    mov     %ebp, 28(%eax)
    mov     4(%ebp), 32(%eax)   # jump to the return address later
    pushf
    pop     36(%eax)
    mov     %cr0, 40(%eax)
    mov     %cr2, 44(%eax)
    mov     %cr3, 48(%eax)
    mov     %cr4, 52(%eax)
    mov     %cs, 56(%eax)
    mov     %ds, 60(%eax)
    mov     %es, 64(%eax)
    mov     %fs, 68(%eax)
    mov     %gs, 72(%eax)
    mov     %ss, 76(%eax)
    mov     $1, %eax
    leave
    ret

restore_regs:
    mov 4(%esp), %eax
    mov 4(%eax), %ebx
    mov 8(%eax), %ecx
    mov 12(%eax), %edx
    mov 16(%eax), %esi
    mov 20(%eax), %edi
    push 36(%eax)
    popf
    mov 40(%eax), %cr0
    mov 44(%eax), %cr2
    mov 48(%eax), %cr3
    mov 52(%eax), %cr4
    // mov 56(%eax), %cs
    mov 60(%eax), %ds
    mov 64(%eax), %es
    mov 68(%eax), %fs
    mov 72(%eax), %gs
    mov 76(%eax), %ss
    mov 24(%eax), %esp
    mov 28(%eax), %ebp
    jmp 32(%eax)
