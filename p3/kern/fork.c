/** @file fork.c
 *  @brief Implements fork.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <scheduler.h>
#include <proc.h>
#include <simics.h>
#include <proc.h>
#include <syscall.h>
#include <context_switch.h>
#include <asm.h>
#include <cr.h>
#include <vm.h>
#include <string.h>
#include "driver.h"

/** @brief Creates a new thread in the current task.
 *
 *  The new task contains a single thread which is a copy of the thread
 *  invoking fork() except for the return value of the system call.
 *
 *  @return The ID of the new task’s thread to the invoking thread, 0 to the
 *  newly created thread.
 */
int fork()
{
    tcb_t *old_tcb;
    hashtable_get(&tcbs, gettid(), (void**)&old_tcb);

    disable_interrupts();

    unsigned old_esp0 = get_esp0();

    int tid = proc_new_process();
    if (tid < 0) {
        return -1;
    }

    if (store_regs(&old_tcb->regs, old_esp0)) {
        cur_tid = tid;

        unsigned new_esp0 = get_esp0();
        set_esp0(old_esp0);

        memcpy((void *)(new_esp0 - KERNEL_STACK_SIZE), (void *)(old_esp0 - KERNEL_STACK_SIZE), KERNEL_STACK_SIZE);

        set_cr3((unsigned)vm_copy());

        enable_interrupts();
        return 0;
    } else {
        cur_tid = old_tcb->tid;

        // we are returning from a timer interrupt
        notify_interrupt_complete();
        enable_interrupts();
        
        return tid;
    }
}
