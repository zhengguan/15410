/** @file get_ticks.S
 *  @brief The get_ticks system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include<syscall_int.h>

.globl get_ticks

get_ticks:
    push    %ebp
    mov     %esp, %ebp
    int     $GET_TICKS_INT
    mov     %ebp, %esp
    pop     %ebp
    ret
