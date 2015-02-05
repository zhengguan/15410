.globl remove_pages

remove_pages:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    mov     8(%ebp), %esi
    int     $0x4a
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
