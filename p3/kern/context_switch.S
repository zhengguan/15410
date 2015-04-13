/** @file context_switch.S
 *  @brief Function definitions for context switching.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

.globl store_regs
store_regs:
    mov     4(%esp), %ecx       # move regs struct to ecx

    mov     8(%esp), %edx       # mov esp0 to edx
    subl    %esp, %edx          # calculate current esp offset
    subl    $4, %edx            # include return address in calculation
    mov     %edx, 12(%ecx)      # save the esp offset

    mov     8(%esp), %edx       # mov esp0 to edx
    subl    %ebp, %edx          # calculate ebp offset
    mov     %edx, 16(%ecx)      # save the ebp offset

    mov     %ebx, (%ecx)        # save %ebx
    mov     %esi, 4(%ecx)       # save %esi
    mov     %edi, 8(%ecx)       # save %edi
    mov     (%esp), %edx        # mov return address to %edx
    mov     %edx, 20(%ecx)      # save return address as eip
    pushfl                      # push the eflags
    popl    24(%ecx)            # pop the eflags to the struct
    mov     %cr2, %edx          # mov %cr2 to %edx
    movl    %edx, 28(%ecx)      # save %cr2
    mov     %cr3, %edx          # mov %cr3 to %edx
    movl    %edx, 32(%ecx)      # save %cr3
    movl     $1, %eax           # on first return, return 1
    ret                         # return

.globl restore_regs
restore_regs:
    call sim_breakpoint
    movl     4(%esp), %ecx      # move regs struct to %ecx

    mov     8(%esp), %edx       # mov ebp offset to %edx
    subl    16(%ecx), %edx      # calculate ebp
    mov     %edx, %ebp          # restore %ebp

    mov     8(%esp), %edx       # move esp offset to %edx
    subl    12(%ecx), %edx      # calculate esp
    mov     %edx, %esp          # restore esp

    mov     (%ecx), %ebx        # restore %ebx
    mov     4(%ecx), %esi       # restore %esi
    mov     8(%ecx), %edi       # restore %edi
    pushl   24(%ecx)            # push eflags from struct
    popfl                       # pop eflags to reg
    movl    28(%ecx), %edx      # move cr2 to edx
    mov     %edx, %cr2          # restore cr2
    movl    32(%ecx), %edx      # move cr3 to edx
    mov     %edx, %cr3          # restore cr3
    mov     $0, %eax            # return 0 on second return
    jmp     20(%ecx)            # jump to stored esi
