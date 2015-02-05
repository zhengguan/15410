.globl set_status

set_status:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    mov     8(%ebp), %esi
    int     $0x59
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
