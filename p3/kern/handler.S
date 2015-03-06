/** @file handler.S
 *  @brief The system-call interrupt handlers.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

/* Life cycle */


/* Thread management */

.globl gettid_int
gettid_int:
    call    gettid
    iret


/* Memory management */

.globl new_pages_int
new_pages_int:
    pushl   4(%esi)
    pushl   (%esi)
    call    new_pages
    addl    $8, %esp
    iret
    
.globl remove_pages_int
remove_pages_int:
    push    %esi
    call    remove_pages
    iret


/* Console I/O */


/* Miscellaneous system calls */
