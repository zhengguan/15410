/** @file readfile.S
 *  @brief The readfile system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include<syscall_int.h>

.globl readfile

readfile:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    lea     8(%ebp), %esi
    int     $READFILE_INT
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
