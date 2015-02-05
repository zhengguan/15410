.globl fork

fork:
    push    %ebp
    mov     %esp, %ebp
    int     $0x41
    mov     %ebp, %esp
    pop     %ebp
    ret
