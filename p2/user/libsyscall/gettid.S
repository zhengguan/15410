.globl gettid

gettid:
    push    %ebp
    mov     %esp, %ebp
    int     $0x48
    mov     %ebp, %esp
    pop     %ebp
    ret
