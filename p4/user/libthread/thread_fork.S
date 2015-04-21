/** @file thread_fork.S
 *  @brief The thread_fork system call wrapper.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <syscall_int.h>

.global thread_fork

thread_fork:
    push    %ebx                # save callee save register
    push    %edi                # save callee save register
    mov     12(%esp), %ebx      # get the new thread's %eip
    mov     16(%esp), %edi      # get the new thread's %esp
    int     $THREAD_FORK_INT    # thread_fork
    test    %eax, %eax          # check which thread we're in
    jne     OLD_THREAD          # if we're in the old thread, jmp
    mov     %edi, %esp          # set the new %esp
    mov     $0, %ebp            # set the old %ebp to 0
    jmp     %ebx                # jmp to the new %eip
    OLD_THREAD:
    pop     %edi                # restore callee save register
    pop     %ebx                # restore callee save register
    ret
