/** @file new_kernel_thread.S
 *  @brief This file implements the function new_kernel_thread.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include<syscall_int.h>

.global new_kernel_thread

new_kernel_thread:
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

