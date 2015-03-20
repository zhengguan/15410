/** @file context_switch_asm.S
 *  @brief Implementation of context_switch_asm
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

.globl store_regs
store_regs:
    mov     4(%esp), %eax
    mov     %ebx, (%eax)
    mov     %esi, 4(%eax)
    mov     %edi, 8(%eax)
    mov     %esp, %ecx
    addl    $4, %ecx
    mov     %ecx, 12(%eax)
    mov     %ebp, 16(%eax)
    mov     (%esp), %ecx
    mov     %ecx, 20(%eax)
    pushf
    pop     24(%eax)
    mov     %cr0, %ecx
    mov     %ecx, 28(%eax)
    mov     %cr2, %ecx
    mov     %ecx, 32(%eax)
    mov     %cr3, %ecx
    mov     %ecx, 36(%eax)
    mov     %cr4, %ecx
    mov     %ecx, 40(%eax)
    mov     %cs, 44(%eax)
    mov     %ds, 48(%eax)
    mov     %es, 52(%eax)
    mov     %fs, 56(%eax)
    mov     %gs, 60(%eax)
    mov     %ss, 64(%eax)
    mov     $1, %eax
    ret
  RETURN:
    mov $1, %esp
    leave
    ret

.globl restore_regs
restore_regs:
    mov     4(%esp), %eax
    mov     (%eax), %ebx
    mov     8(%eax), %esi
    mov     12(%eax), %edi
    mov     16(%eax), %esp
    mov     20(%eax), %ebp
    push    24(%eax)
    popf
    mov     28(%eax), %ecx
    mov     %ecx, %cr0
    mov     32(%eax), %ecx
    mov     %ecx, %cr2
    mov     36(%eax), %ecx
    mov     %ecx, %cr3
    mov     40(%eax), %ecx
    mov     %ecx, %cr4
    // mov  44(%eax), %cs
    mov     48(%eax), %ds
    mov     52(%eax), %es
    mov     56(%eax), %fs
    mov     60(%eax), %gs
    mov     64(%eax), %ss
    mov     $0, %eax
    jmp     32(%eax)
