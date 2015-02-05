.globl sleep

sleep:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    mov     8(%ebp), %esi
    int     $0x4b
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
