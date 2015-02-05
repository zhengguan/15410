.globl task_vanish

task_vanish:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    mov     8(%ebp), %esi
    int     $0x57
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
