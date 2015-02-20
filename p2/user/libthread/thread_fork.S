/** @file thread_fork.S
 *  @brief The thread_fork system call wrapper.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include<syscall_int.h>

.global thread_fork

thread_fork:
    mov     4(%esp), %ecx       # get the new thread's %eip
    mov     8(%esp), %edx       # get the new thread's %esp
    int     $THREAD_FORK_INT    # thread_fork
    test    %eax, %eax          # check which thread we're in
    jne     RETURN              # if we're in the old thread, ret
    mov     %edx, %esp          # set the new %esp
    mov     $0, %ebp            # set the old %ebp to 0
    jmp     %ecx                # jmp to the new %eip
    RETURN:
    ret
