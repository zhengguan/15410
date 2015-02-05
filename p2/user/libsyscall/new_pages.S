.globl new_pages

new_pages:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    lea     8(%ebp), %esi
    int     $0x49
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
