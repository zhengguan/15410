.globl make_runnable

make_runnable:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    mov     8(%ebp), %esi
    int     $0x47
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
