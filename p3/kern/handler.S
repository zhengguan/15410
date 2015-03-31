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
    call    set_kernel_segs
    call    timer_handler
    call    set_user_segs
    iret

.globl keyboard_int
keyboard_int:
    call    set_kernel_segs
    call    keyboard_handler
    call    set_user_segs
    iret

/* Life cycle */

.globl fork_int
fork_int:
    call    set_kernel_segs
    call    fork
    push    %eax
    call    set_user_segs
    pop     %eax
    iret

.globl exec_int
exec_int:
    call    set_kernel_segs
    pushl   4(%esi)
    pushl   (%esi)
    call    exec
    addl    $8, %esp
    push    %eax
    call    set_user_segs
    pop     %eax
    iret

.globl set_status_int
set_status_int:
    call    set_kernel_segs
    pushl   %esi
    call    set_status
    addl    $4, %esp
    push    %eax
    call    set_user_segs
    pop     %eax
    iret

/* Thread management */

.globl gettid_int
gettid_int:
    call    set_kernel_segs
    call    gettid
    push    %eax
    call    set_user_segs
    pop     %eax
    iret

.globl yield_int
yield_int:
    call    set_kernel_segs
    pushl   %esi
    call    yield
    addl    $4, %esp
    push    %eax
    call    set_user_segs
    pop     %eax
    iret

.globl deschedule_int
deschedule_int:
    call    set_kernel_segs
    pushl   %esi
    call    deschedule
    addl    $4, %esp
    push    %eax
    call    set_user_segs
    pop     %eax
    iret

.globl make_runnable_int
make_runnable_int:
    call    set_kernel_segs
    pushl   %esi
    call    make_runnable
    addl    $4, %esp
    push    %eax
    call    set_user_segs
    pop     %eax
    iret

.globl get_ticks_int
get_ticks_int:
    call    set_kernel_segs
    call    get_ticks
    push    %eax
    call    set_user_segs
    pop     %eax
    iret

.globl sleep_int
sleep_int:
    call    set_kernel_segs
    pushl   %esi
    call    sleep
    addl    $4, %esp
    push    %eax
    call    set_user_segs
    pop     %eax
    iret

/* Memory management */

.globl new_pages_int
new_pages_int:
    call    set_kernel_segs
    pushl   4(%esi)
    pushl   (%esi)
    call    new_pages
    addl    $8, %esp
    push    %eax
    call    set_user_segs
    pop     %eax
    iret

.globl remove_pages_int
remove_pages_int:
    call    set_kernel_segs
    pushl   %esi
    call    remove_pages
    addl    $4, %esp
    push    %eax
    call    set_user_segs
    pop     %eax
    iret


/* Console I/O */


/* Miscellaneous system calls */

.globl swexn_int
swexn_int:
    call    set_kernel_segs
    pushl   16(%esi)
    pushl   12(%esi)
    pushl   8(%esi)
    pushl   4(%esi)
    pushl   (%esi)
    call    kern_swexn
    addl    $20, %esp
    push    %eax
    call    set_user_segs
    pop     %eax
    iret

.globl exn_handler_complete_int
exn_handler_complete_int:
    call    set_kernel_segs
    call    exn_handler_complete
    push    %eax
    call    set_user_segs
    pop     %eax
    iret
