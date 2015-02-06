/** @file getchar.S.
 *  @brief The getchar system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include<syscall_int.h>

.globl getchar

getchar:
    push    %ebp
    mov     %esp, %ebp
    int     $GETCHAR_INT
    mov     %ebp, %esp
    pop     %ebp
    ret
