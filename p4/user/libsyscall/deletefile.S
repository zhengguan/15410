/** @file deletefile.S
 *  @brief The deletefile system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include<syscall_int.h>

.globl deletefile

deletefile:
    push    %ebp
    mov     %esp, %ebp
    push    %esi
    mov     8(%ebp), %esi
    int     $DELETEFILE_INT
    pop     %esi
    mov     %ebp, %esp
    pop     %ebp
    ret
