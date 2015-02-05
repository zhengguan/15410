.globl print

print:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    lea     8(%ebp), %esi
    int     $0x4e
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
