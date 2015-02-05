.globl swexn

swexn:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    lea     8(%ebp), %esi
    int     $0x74
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
