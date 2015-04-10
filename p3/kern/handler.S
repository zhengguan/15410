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

.globl thread_fork_int
thread_fork_int:
    call    set_kernel_segs
    call    thread_fork
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

.globl vanish_int
vanish_int:
    call    set_kernel_segs
    call    vanish
    call    set_user_segs
    iret

.globl wait_int
wait_int:
    call    set_kernel_segs
    pushl   %esi
    call    int_check
    test    %eax, %eax
    js      wait_fail
    call    wait
wait_fail:
    addl    $4, %esp
    pushl   %eax
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
    call    int_check
    test    %eax, %eax
    js      deschedule_fail
    call    deschedule
deschedule_fail:
    addl    $4, %esp
    pushl   %eax
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

.globl swexn_int
swexn_int:
    pushl   12(%esi)
    pushl   8(%esi)
    pushl   4(%esi)
    pushl   (%esi)
    call    set_kernel_segs
    call    swexn
    addl    $16, %esp
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

.globl readline_int
readline_int:
    call    set_kernel_segs
    pushl   4(%esi)
    pushl   (%esi)
    call    str_check
    test    %eax, %eax
    js      print_fail
    call    readline
readline:
    addl    $8, %esp
    push    %eax
    call    set_user_segs
    pop     %eax
    iret

.globl print_int
print_int:
    call    set_kernel_segs
    pushl   4(%esi)
    pushl   (%esi)
    call    str_check
    test    %eax, %eax
    js      print_fail
    call    print
print_fail:
    addl    $8, %esp
    push    %eax
    call    set_user_segs
    pop     %eax
    iret

.globl set_term_color_int
set_term_color_int:
    call    set_kernel_segs
    pushl   %esi
    call    set_term_color
    addl    $4, %esp
    push    %eax
    call    set_user_segs
    pop     %eax
    iret

.globl set_cursor_pos_int
set_cursor_pos_int:
    call    set_kernel_segs
    pushl   4(%esi)
    pushl   (%esi)
    call    set_cursor_pos
    addl    $8, %esp
    push    %eax
    call    set_user_segs
    pop     %eax
    iret

.globl get_cursor_pos_int
get_cursor_pos_int:
    call    set_kernel_segs
    pushl   4(%esi)
    call    int_check
    test    %eax, %eax
    js      get_cursor_pos_row_fail
    pushl   (%esi)
    call    int_check
    test    %eax, %eax
    js      get_cursor_pos_col_fail
    call    get_cursor_pos
get_cursor_pos_col_fail:
    addl    $4, %esp
get_cursor_pos_row_fail:
    addl    $4, %esp
    push    %eax
    call    set_user_segs
    pop     %eax
    iret

/* Miscellaneous */

.globl readfile_int
readfile_int:
    call    set_kernel_segs
    pushl   12(%esi)
    pushl   8(%esi)
    pushl   4(%esi)
    pushl   (%esi)
    call    readfile
    addl    $16, %esp
    push    %eax
    call    set_user_segs
    pop     %eax
    iret

.globl halt_int
halt_int:
    call sim_halt
    hlt
    iret

/* "Special" */

.globl misbehave_int
misbehave_int:
    iret
