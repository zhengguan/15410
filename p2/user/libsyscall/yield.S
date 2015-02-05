.globl yield

yield:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    mov     8(%ebp), %esi
    int     $0x45
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
