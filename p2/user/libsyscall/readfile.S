.globl readfile

readfile:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    lea     8(%ebp), %esi
    int     $0x62
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
