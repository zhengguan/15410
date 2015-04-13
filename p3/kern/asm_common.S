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
    mov     4(%esp), %esp   # set passed ureg as stack
    addl    $8, %esp        # ignore cause/cr2
    pop     %ds             # restore ds
    pop     %es             # restore es
    pop     %fs             # restore fs
    pop     %gs             # restore gs
    popa                    # restore general purpose regs
    addl    $4, %esp        # ignore error code
    iret                    # restore remaining regs

.globl jmp_ureg_user
jmp_ureg_user:
    call set_user_segs                  # set user data segments
    mov     4(%esp), %esp               # set passed ureg as stack
    addl    $24, %esp                   # ignore cause, cr2, and segments
    mov     %esp, %eax                  # fix dummy esp to ensure popa works
    addl    $32, %eax                   # get dummy esp in right place
    mov     %eax, 12(%esp)              # set zero to dummy esp
    popa                                # restore general purpose registers
    addl    $4, %esp                    # ignore error code
    movl    $SEGSEL_USER_CS, 4(%esp)    # setup cs for iret
    movl    $USER_EFLAGS, 8(%esp)       # setup eflags for iret
    movl    $SEGSEL_USER_DS, 16(%esp)   # setup ss for iret
    iret                                # restore remaining registers

.globl jmp_user
jmp_user:
    mov     %esp, %eax          # save esp for future use
    push    $SEGSEL_USER_DS     # push ss
    push    8(%eax)             # push passed esp
    push    $USER_EFLAGS        # push eflags
    push    $SEGSEL_USER_CS     # push cs
    push    4(%eax)             # push passed eip
.globl iret_user
iret_user:
    call    set_user_segs       # set user data segments
    mov     $0, %edi            # zero out general purpose registers
    mov     $0, %esi
    mov     $0, %ebp
    mov     $0, %ebx
    mov     $0, %edx
    mov     $0, %ecx
    mov     $0, %eax
    iret                        # restore previously pushed registers

.globl set_kernel_segs
set_kernel_segs:
    mov     $SEGSEL_KERNEL_DS, %eax     # store kernel data segment
    jmp     set_segs
.globl set_user_segs
set_user_segs:
    mov     $SEGSEL_USER_DS, %eax       # store user data segment
set_segs:
    mov     %eax, %ds                   # set data segments
    mov     %eax, %es
    mov     %eax, %fs
    mov     %eax, %gs
    ret                                 # return

.globl flush_tlb
flush_tlb:
    mov     %cr3, %eax          # save cr3
    movl    %eax, %cr3          # write to cr3 to flush tlb
    ret                         # return

.globl flush_tlb_entry
flush_tlb_entry:
    mov     4(%esp), %eax       # store argument
    invlpg  (%eax)              # invalidate page in tlb
    ret                         # return