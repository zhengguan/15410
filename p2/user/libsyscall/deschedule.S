.globl deschedule

deschedule:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    mov     8(%ebp), %esi
    int     $0x46
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
