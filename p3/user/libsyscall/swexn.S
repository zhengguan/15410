/** @file swexn.S
 *  @brief The swexn system-call stub.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <syscall_int.h>

.globl swexn

swexn:
    push    %esi
    lea     8(%esp), %esi
    int     $SWEXN_INT
    pop     %esi
    ret