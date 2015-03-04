/** @file exec_run.S
 *  @brief
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

.globl exec_run

exec_run:
    mov 8(%esp) %ecx
    mov 4(%esp) %esp
    jmp %ecx
