/** @file asm_syscall.c
 *  @brief The system-call wrappers.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

/* Life cycle */
    
.globl asm_exec
asm_exec:
    push    4(%esi)
    push    (%esi)
    call    exec
    iret


/* Thread management */

.globl asm_gettid
asm_gettid:
    call    gettid
    iret


/* Memory management */

.globl asm_new_pages
asm_new_pages:
    pushl   4(%esi)
    pushl   (%esi)
    call    new_pages
    addl    $8, %esp
    iret
    
.globl asm_remove_pages
asm_remove_pages:
    push    %esi
    call    remove_pages
    iret


/* Console I/O */


/* Miscellaneous system calls */
