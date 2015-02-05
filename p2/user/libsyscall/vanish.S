.globl vanish

vanish:
    push    %ebp
    mov     %esp, %ebp
    int     $0x60
    mov     %ebp, %esp
    pop     %ebp
    ret
