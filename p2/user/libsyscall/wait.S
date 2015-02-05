.globl wait

wait:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    mov     8(%ebp), %esi
    int     $0x44
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
