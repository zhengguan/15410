.globl set_term_color

set_term_color:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    mov     8(%ebp), %esi
    int     $0x4f
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
