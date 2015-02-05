.globl get_ticks

get_ticks:
    push    %ebp
    mov     %esp, %ebp
    int     $0x53
    mov     %ebp, %esp
    pop     %ebp
    ret
