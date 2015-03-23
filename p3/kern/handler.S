/** @file handler.S
 *  @brief The system-call interrupt handlers.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

/* Drivers */

.globl timer_handler_int
timer_handler_int:
    pusha
    call    timer_handler
    popa
    iret

.globl keyboard_int
keyboard_int:
    pusha
    call    keyboard_handler
    popa
    iret

/* Life cycle */

.globl fork_int
fork_int:
    call    fork
    iret

.globl exec_int
exec_int:
    pushl   4(%esi)
    pushl   (%esi)
    call    exec
    addl    $8, %esp
    iret

.globl set_status_int
set_status_int:
    pushl   %esi
    call    set_status
    addl    $4, %esp
    iret

/* Thread management */

.globl gettid_int
gettid_int:
    call    gettid
    iret

.globl yield_int
yield_int:
    pushl   %esi
    call    yield
    addl    $4, %esp
    iret

.globl deschedule_int
deschedule_int:
    pushl   %esi
    call    deschedule
    addl    $4, %esp
    iret

.globl make_runnable_int
make_runnable_int:
    pushl   %esi
    call    make_runnable
    addl    $4, %esp
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
    pushl   %esi
    call    remove_pages
    addl    $4, %esp
    iret


/* Console I/O */


/* Miscellaneous system calls */
