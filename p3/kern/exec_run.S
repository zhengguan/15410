/** @file exec_run.S
 *  @brief Runs a user program.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

.globl exec_run
exec_run:
    cli
    movl $0, %ebp
    addl $4, %esp   # Ignore exception address
    pop %ds         # Update the data segment
    pop %es         # Update the extra segment
    iret
