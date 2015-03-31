/** @file context_switch.S
 *  @brief Function definitions for context switching.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

.globl store_regs
store_regs:
    mov     4(%esp), %ecx

    mov     8(%esp), %edx
    subl    %esp, %edx
    subl    $4, %edx
    mov     %edx, 12(%ecx)

    mov     %ebx, (%ecx)
    mov     %esi, 4(%ecx)
    mov     %edi, 8(%ecx)
    mov     %ebp, 16(%ecx)
    mov     (%esp), %edx
    mov     %edx, 20(%ecx)
    pushfl
    popl     24(%ecx)
    mov     %cr0, %edx
    movl    %edx, 28(%ecx)
    mov     %cr2, %edx
    movl    %edx, 32(%ecx)
    mov     %cr3, %edx
    movl    %edx, 36(%ecx)
    mov     %cr4, %edx
    movl    %edx, 40(%ecx)
    movl     $1, %eax
    ret

.globl restore_regs
restore_regs:
    movl     4(%esp), %ecx

    mov     8(%esp), %edx
    subl    12(%ecx), %edx
    mov     %edx, %esp

    mov     (%ecx), %ebx
    mov     4(%ecx), %esi
    mov     8(%ecx), %edi
    mov     16(%ecx), %ebp
    pushl    24(%ecx)
    popfl
    movl    28(%ecx), %edx
    mov     %edx, %cr0
    movl    32(%ecx), %edx
    mov     %edx, %cr2
    movl    36(%ecx), %edx
    mov     %edx, %cr3
    movl    40(%ecx), %edx
    mov     %edx, %cr4
    mov     $0, %eax
    jmp     20(%ecx)
