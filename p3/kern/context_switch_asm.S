/** @file context_switch_asm.S
 *  @brief Implementation of context_switch_asm
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

.globl store_regs
store_regs:
    call    get_esp0
    mov     4(%esp), %ecx
    mov     %eax, 68(%ecx)
    
    subl    %esp, %eax
    subl    $4, %eax
    mov     %eax, 12(%ecx)
    
    mov     %ebx, (%ecx)
    mov     %esi, 4(%ecx)
    mov     %edi, 8(%ecx)
    mov     %ebp, 16(%ecx)
    mov     (%esp), %edx
    mov     %edx, 20(%ecx)
    pushf
    pop     24(%ecx)
    mov     %cr0, %edx
    mov     %edx, 28(%ecx)
    mov     %cr2, %edx
    mov     %edx, 32(%ecx)
    mov     %cr3, %edx
    mov     %edx, 36(%ecx)
    mov     %cr4, %edx
    mov     %edx, 40(%ecx)
    mov     %cs, 44(%ecx)
    mov     %ds, 48(%ecx)
    mov     %es, 52(%ecx)
    mov     %fs, 56(%ecx)
    mov     %gs, 60(%ecx)
    mov     %ss, 64(%ecx)    
    mov     $1, %eax
    ret

.globl restore_regs
restore_regs:
    mov     4(%esp), %ecx
    
    mov     68(%ecx), %edx
    subl    12(%ecx), %edx
    mov     %edx, %esp
    
    mov     (%ecx), %ebx
    mov     4(%ecx), %esi
    mov     8(%ecx), %edi
    mov     16(%ecx), %ebp
    push    24(%ecx)
    popf
    mov     28(%ecx), %edx
    mov     %edx, %cr0
    mov     32(%ecx), %edx
    mov     %edx, %cr2
    mov     36(%ecx), %edx
    mov     %edx, %cr3
    mov     40(%ecx), %edx
    mov     %edx, %cr4
    // mov  44(%ecx), %cs
    mov     48(%ecx), %ds
    mov     52(%ecx), %es
    mov     56(%ecx), %fs
    mov     60(%ecx), %gs
    mov     64(%ecx), %ss
    push    %ecx
    push    68(%ecx)
    call    set_esp0
    addl    $4, %esp
    pop     %ecx
    mov     $0, %eax
    jmp     20(%ecx)
