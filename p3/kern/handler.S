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
    call set_kernel_segs
    call    timer_handler
    call set_user_segs
    iret

.globl keyboard_int
keyboard_int:
    call set_kernel_segs
    call    keyboard_handler
    call set_user_segs
    iret

/* Life cycle */

.globl fork_int
fork_int:
    call set_kernel_segs
    call    fork
    push %eax
    call set_user_segs
    pop %eax
    iret

.globl exec_int
exec_int:
    pushl   4(%esi)
    pushl   (%esi)
    call set_kernel_segs
    call    exec
    push %eax
    call set_user_segs
    pop %eax
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
    call set_kernel_segs
    call    gettid
    push %eax
    call set_user_segs
    pop %eax
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
    call set_kernel_segs
    call    new_pages
    push %eax
    call set_user_segs
    pop %eax
    addl    $8, %esp
    iret

.globl remove_pages_int
remove_pages_int:
    pushl   %esi
    call set_kernel_segs
    call    remove_pages
    push %eax
    call set_user_segs
    pop %eax
    addl    $4, %esp
    iret


/* Console I/O */


/* Miscellaneous system calls */

.globl swexn_int
swexn_int:
    pushl   16(%esi)
    pushl   12(%esi)
    pushl   8(%esi)
    pushl   4(%esi)
    pushl   (%esi)
    call set_kernel_segs
    call    kern_swexn
    push %eax
    call set_user_segs
    pop %eax
    addl    $20, %esp
    iret

.globl exn_handler_complete_int
exn_handler_complete_int:
    call set_kernel_segs
    call    exn_handler_complete
    push %eax
    call set_user_segs
    pop %eax
    iret