/** @file handler.S
 *  @brief The system-call interrupt handlers.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

/* Life cycle */

#include <seg.h>


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

.globl timer_handler_int
timer_handler_int:
    call    timer_handler
    iret

.globl keyboard_int
keyboard_int:
    call    keyboard_handler
    iret

.globl fork_int
fork_int:
    mov $SEGSEL_KERNEL_DS, %ecx
    mov %ecx, %ds
    call    fork
    mov $SEGSEL_USER_DS, %ecx
    mov %ecx, %ds
    iret

.globl exec_int
exec_int:
    mov $SEGSEL_KERNEL_DS, %ecx
    mov %ecx, %ds
    pushl   4(%esi)
    pushl   (%esi)
    call    exec
    mov $SEGSEL_USER_DS, %ecx
    mov %ecx, %ds
    addl    $8, %esp
    iret


/* Console I/O */


/* Miscellaneous system calls */
