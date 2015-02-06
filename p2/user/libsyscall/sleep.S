/** @file sleep.S.
 *  @brief The sleep system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include<syscall_int.h>

.globl sleep

sleep:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    mov     8(%ebp), %esi
    int     $SLEEP_INT
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
