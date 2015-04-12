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
    mov     $0, %ecx
    mov     $0, %edx
    mov     $0, %eax
    iret

.globl keyboard_int
keyboard_int:
    call    set_kernel_segs
    call    keyboard_handler
    call    set_user_segs
    mov     $0, %ecx
    mov     $0, %edx
    mov     $0, %eax
    iret

/* Life cycle */

.globl fork_int
fork_int:
    call    set_kernel_segs
    call    fork
    push    %eax
    call    set_user_segs
    pop     %eax
    mov     $0, %ecx
    mov     $0, %edx
    iret

.globl thread_fork_int
thread_fork_int:
    call    set_kernel_segs
    call    thread_fork
    push    %eax
    call    set_user_segs
    pop     %eax
    mov     $0, %ecx
    mov     $0, %edx
    iret

.globl exec_int
exec_int:
    call    set_kernel_segs
    subl    $4, %esp
    pushl   %esi
    pushl   $8
    call    buf_lock
    test    %eax, %eax
    js      exec_esi_fail
    pushl   4(%esi)
    pushl   (%esi)
    call    exec
    addl    $8, %esp
    mov     %eax, 8(%esp)
    call    buf_unlock
    mov     8(%esp), %eax
exec_esi_fail:
    addl    $12, %esp
    push    %eax
    call    set_user_segs
    pop     %eax
    mov     $0, %ecx
    mov     $0, %edx
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
    mov     $0, %ecx
    mov     $0, %edx
    iret

.globl vanish_int
vanish_int:
    call    set_kernel_segs
    call    vanish
    #shouldn't get here

.globl wait_int
wait_int:
    call    set_kernel_segs     # set kernel data segments
    subl    $4, %esp            # allocate space for return value
    pushl   %esi                # push the status_ptr
    call    int_lock            # check the status_ptr
    test    %eax, %eax          # test if the check failed
    js      wait_fail           # if so, jump
    call    wait                # call wait
    mov     %eax, 4(%esp)       # save the return value
    call    int_unlock          # unlock the status_ptr
    mov     4(%esp), %eax       # restore the return value
wait_fail:
    addl    $8, %esp            # remove the status_ptr and ret from the stack
    pushl   %eax                # save the return value
    call    set_user_segs       # set user data segments
    pop     %eax                # restore the return value
    mov     $0, %ecx
    mov     $0, %edx
    iret                        # return from the interrupt

/* Thread management */

.globl gettid_int
gettid_int:
    call    set_kernel_segs
    call    gettid
    push    %eax
    call    set_user_segs
    pop     %eax
    mov     $0, %ecx
    mov     $0, %edx
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
    mov     $0, %ecx
    mov     $0, %edx
    iret

.globl deschedule_int
deschedule_int:
    call    set_kernel_segs     # set kernel data segments
    subl    $4, %esp            # allocate space for return value
    pushl   %esi                # push the flag
    call    int_lock            # check the flag
    test    %eax, %eax          # test if the flag is valid
    js      deschedule_fail     # if not, jump
    call    deschedule          # call deschedule
    mov     %eax, 4(%esp)       # save the return value
    call    int_unlock          # unlock the flag
    mov     4(%esp), %eax       # restore the return value
deschedule_fail:
    addl    $8, %esp            # remove flag and ret from stack
    pushl   %eax                # save the return value
    call    set_user_segs       # set user data segments
    pop     %eax                # restore the return value
    mov     $0, %ecx
    mov     $0, %edx
    iret                        # return from the interrupt

.globl make_runnable_int
make_runnable_int:
    call    set_kernel_segs
    pushl   %esi
    call    make_runnable
    addl    $4, %esp
    push    %eax
    call    set_user_segs
    pop     %eax
    mov     $0, %ecx
    mov     $0, %edx
    iret

.globl get_ticks_int
get_ticks_int:
    call    set_kernel_segs
    call    get_ticks
    push    %eax
    call    set_user_segs
    pop     %eax
    mov     $0, %ecx
    mov     $0, %edx
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
    mov     $0, %ecx
    mov     $0, %edx
    iret

.globl swexn_int
swexn_int:
    call    set_kernel_segs
    subl    $4, %esp
    pushl   %esi
    pushl   $16
    call    buf_lock
    test    %eax, %eax
    js      swexn_esi_fail
    pushl   12(%esi)
    pushl   8(%esi)
    pushl   4(%esi)
    pushl   (%esi)
    call    swexn
    addl    $16, %esp
    mov     %eax, 8(%esp)
    call    buf_unlock
    mov     8(%esp), %eax
swexn_esi_fail:
    addl    $12, %esp
    push    %eax
    call    set_user_segs
    pop     %eax
    mov     $0, %ecx
    mov     $0, %edx
    iret

/* Memory management */

.globl new_pages_int
new_pages_int:
    call    set_kernel_segs
    subl    $4, %esp
    push    %esi
    push    $8
    call    buf_lock
    test %eax, %eax
    js      new_pages_esi_fail
    pushl   4(%esi)
    pushl   (%esi)
    call    new_pages
    addl    $8, %esp
    mov     %eax, 8(%esp)
    call    buf_unlock
    mov     8(%esp), %eax
new_pages_esi_fail:
    addl    $12, %esp
    push    %eax
    call    set_user_segs
    pop     %eax
    mov     $0, %ecx
    mov     $0, %edx
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
    mov     $0, %ecx
    mov     $0, %edx
    iret


/* Console I/O */

.globl readline_int
readline_int:
    call    set_kernel_segs     # set kernel data segments
    subl    $4, %esp            # allocate space for return value
    push    %esi                # push esi
    push    $8                  # push total arg len
    call    buf_lock            # lock esi
    test    %eax, %eax          # test if the lock was successful
    js      readline_esi_fail   # if not, jump
    pushl   4(%esi)             # push the buf
    pushl   (%esi)              # push the len
    call    buf_lock            # check the buf
    test    %eax, %eax          # test if the buf is valid
    js      readline_fail       # jump if not
    call    readline            # call readline
    mov     %eax, 16(%esp)      # save the return value
    call    buf_unlock          # unlock the buf
    mov     16(%esp), %eax      # restore the return value
readline_fail:
    addl    $8, %esp            # remove the args from stack
    mov     %eax, 8(%esp)       # save the return value
    call    buf_unlock          # unlock esi
    mov     8(%esp), %eax       # restore the return value
readline_esi_fail:
    addl    $12, %esp           # remove ret, esi, and arg len from stack
    push    %eax                # save the return value
    call    set_user_segs       # set user data segments
    pop     %eax                # restore the return value
    mov     $0, %ecx
    mov     $0, %edx
    iret                        # return from the interrupt

.globl print_int
print_int:
    call    set_kernel_segs     # set kernel data segments
    subl    $4, %esp            # allocate space for return value
    push    %esi                # push esi
    push    $8                  # push total arg len
    call    buf_lock            # lock esi
    test    %eax, %eax          # test if the lock was successful
    js      print_esi_fail      # if not, jump
    pushl   4(%esi)             # push the buf
    pushl   (%esi)              # push the len
    call    buf_lock            # check the buf
    test    %eax, %eax          # test to see if check failed
    js      print_fail          # jump if it failed
    call    print               # call print
    mov     %eax, 16(%esp)      # save the return value
    call    buf_unlock          # unlock the buf
    mov     16(%esp), %eax      # restore the return value
print_fail:
    addl    $8, %esp            # remove the args from stack
    mov     %eax, 8(%esp)       # save the return value
    call    buf_unlock          # unlock esi
    mov     8(%esp), %eax       # restore the return value
print_esi_fail:
    addl    $12, %esp           # remove ret, esi, and arg len from stack
    push    %eax                # save the return value
    call    set_user_segs       # set user data segments
    pop     %eax                # restore the return value
    mov     $0, %ecx
    mov     $0, %edx
    iret                        # return from the interrupt

.globl set_term_color_int
set_term_color_int:
    call    set_kernel_segs
    pushl   %esi
    call    set_term_color
    addl    $4, %esp
    push    %eax
    call    set_user_segs
    pop     %eax
    mov     $0, %ecx
    mov     $0, %edx
    iret

.globl set_cursor_pos_int
set_cursor_pos_int:
    call    set_kernel_segs
    subl    $4, %esp
    push    %esi
    push    $8
    call    buf_lock
    test    %eax, %eax
    js      set_cursor_pos_esi_fail
    pushl   4(%esi)
    pushl   (%esi)
    call    set_cursor_pos
    addl    $8, %esp
    mov     %eax, 8(%esp)
    call    buf_unlock
    mov     8(%esp), %eax
set_cursor_pos_esi_fail:
    addl    $12, %esp
    push    %eax
    call    set_user_segs
    pop     %eax
    mov     $0, %ecx
    mov     $0, %edx
    iret

.globl get_cursor_pos_int
get_cursor_pos_int:
    call    set_kernel_segs             # set kernel data segments
    subl    $4, %esp                    # allocate space to store return value
    push    %esi                        # push esi
    push    $8                          # push total arg len
    call    buf_lock                    # lock esi
    test    %eax, %eax                  # check if lock passes
    js      get_cursor_pos_esi_fail     # if not, jump
    pushl   4(%esi)                     # push the col
    call    int_lock                    # check the col
    test    %eax, %eax                  # test if check failed
    js      get_cursor_pos_col_fail     # jump if it failed
    pushl   (%esi)                      # push the row
    call    int_lock                    # check the row
    test    %eax, %eax                  # test if the check failed
    js      get_cursor_pos_row_fail     # jump if it failed
    call    get_cursor_pos              # call the function
    mov     %eax, 16(%esp)              # save the return value
    call    int_unlock                  # unlock the row
    mov     16(%esp), %eax              # restore the return value
get_cursor_pos_row_fail:
    addl    $4, %esp                    # remove the row from the stack
    mov     %eax, 12(%esp)              # save the return value
    call    int_unlock                  # unlock the col
    mov     12(%esp), %eax              # restore the return value
get_cursor_pos_col_fail:
    addl    $4, %esp                    # remove the col from the stack
    mov     %eax, 8(%esp)               # save the return value
    call    buf_unlock                  # unlock the row
    mov     8(%esp), %eax               # restore the return value
get_cursor_pos_esi_fail:
    addl    $12, %esp                   # remove the esi, arg len, and ret
    push    %eax                        # save the return value
    call    set_user_segs               # set user data segments
    pop     %eax                        # restore the return value
    mov     $0, %ecx
    mov     $0, %edx
    iret                                # return from the interrupt


/* Miscellaneous */

.globl readfile_int
readfile_int:
    call    set_kernel_segs         # set kernel data segments
    subl    $4, %esp                # allocate space to store return values
    push    %esi                    # push esi
    push    $16                     # push the total arg len
    call    buf_lock                # lock the esi
    test    %eax, %eax              # check if the lock passes
    js      readfile_esi_fail       # if not, jump
    pushl   (%esi)                  # push filename
    call    str_lock                # check the string
    test    %eax, %eax              # test if check failed
    js      readfile_filename_fail  # jump if it failed
    push    %eax                    # save str len
    pushl   4(%esi)                 # push buf
    pushl   8(%esi)                 # push count
    call    buf_lock                # check the buffer
    test    %eax, %eax              # test if check failed
    js      readfile_buf_fail       # jump if it failed
    pushl   12(%esi)                # push offset
    pushl   8(%esi)                 # push count
    pushl   4(%esi)                 # push buf
    pushl   (%esi)                  # push filename
    call    readfile_int            # call readfile
    addl    $16, %esp               # remove the args from the stack
    mov     %eax, 24(%esp)          # save the return value
    call    buf_unlock              # unlock buf
    mov     24(%esp), %eax
readfile_buf_fail:
    addl    $8, %esp                # remove args from stack
    mov     %eax, 16(%esp)          # save the return value
    call    buf_unlock              # unlock the filename
    mov     16(%esp), %eax          # restore the return value
readfile_filename_fail:
    addl    $8, %esp                # remove the args from the stack
    mov     %eax, 8(%esp)           # save the return value
    call    buf_unlock              # unlock esi
    mov     8(%esp), %eax           # restore the return value
readfile_esi_fail:
    addl    $12, %esp               # remove the esi, arg len, and ret
    push    %eax                    # save the return value
    call    set_user_segs           # set user data segments
    pop     %eax                    # restore the return value
    mov     $0, %ecx
    mov     $0, %edx
    iret                            # return from the interrupt

.globl halt_int
halt_int:
    call sim_halt
    hlt
    mov     $0, %ecx
    mov     $0, %edx
    mov     $0, %eax
    iret

/* "Special" */

.globl misbehave_int
misbehave_int:
    iret
