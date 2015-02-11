/** @file gettid.S
 *  @brief The gettid system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include<syscall_int.h>

.globl gettid

gettid:
    push    %ebp
    mov     %esp, %ebp
    int     $GETTID_INT
    mov     %ebp, %esp
    pop     %ebp
    ret
