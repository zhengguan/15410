/** @file fork.c
 *  @brief Implements fork
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
#include <fork.h>
#include <scheduler.h>
#include <proc.h>
#include <simics.h>
#include <proc.h>
#include <syscall.h>
#include <context_switch_asm.h>
#include <asm.h>

int fork()
{
    tcb_t *old_tcb;
    hashtable_get(&tcbs, gettid(), (void**)&old_tcb);

    disable_interrupts();

    int pid;
    if ((pid = proc_new_process()) < 0) {
        lprintf("fucked up7");
        MAGIC_BREAK;
    }
    MAGIC_BREAK;
    int ret = context_switch_fork_asm(&old_tcb->regs) ? pid : 0;

    enable_interrupts();
    return ret;
}