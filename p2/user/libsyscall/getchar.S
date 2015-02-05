.globl getchar

getchar:
    push    %ebp
    mov     %esp, %ebp
    int     $0x4c
    mov     %ebp, %esp
    pop     %ebp
    ret
