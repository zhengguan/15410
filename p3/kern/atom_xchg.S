/** @file atom_xchg.S
 *  @brief Performs an atomic xchg.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include<syscall_int.h>

.globl atom_xchg

atom_xchg:
    mov     8(%esp), %eax
    mov     4(%esp), %ecx
    xchg    (%ecx), %eax
    ret
