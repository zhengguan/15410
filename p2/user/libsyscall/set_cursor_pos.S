.globl set_cursor_pos

set_cursor_pos:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    mov     8(%ebp), %esi
    int     $0x50
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
