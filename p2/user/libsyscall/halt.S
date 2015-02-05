.globl halt

halt:
    push    %ebp
    mov     %esp, %ebp
    int     $0x55
    mov     %ebp, %esp
    pop     %ebp
    ret
