/** @file swexn.S
 *  @brief The swexn system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include<syscall_int.h>

.globl swexn

swexn:
    push    16(%esp)
    push    16(%esp)
    push    16(%esp)
    push    16(%esp)
    push    $exception_wrapper //push exception_wrapper as additional arg
    push    %esi
    lea     4(%esp), %esi
    int     $SWEXN_INT
    pop     %esi
    addl    $20, %esp
    ret