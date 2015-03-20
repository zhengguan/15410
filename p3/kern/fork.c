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
#include <cr.h>
#include <vm.h>

int fork()
{
    tcb_t *old_tcb;
    hashtable_get(&tcbs, gettid(), (void**)&old_tcb);

    disable_interrupts();

    int tid = 0;

    if (store_regs(&old_tcb->regs)) {
        MAGIC_BREAK;
        if ((tid = proc_new_process()) < 0) {
            lprintf("fucked up7");
            MAGIC_BREAK;
        }

        lprintf("1: %d", tid);

        //may be bad?
        unsigned cr3 = vm_copy();
        lprintf("1.5: %d", tid);
        set_cr3(cr3);

        lprintf("2: %d", tid);

        enable_interrupts();
        return 0;

    } else {
        MAGIC_BREAK;
        enable_interrupts();
        MAGIC_BREAK;
        lprintf("3: %d", tid);
        return tid;
    }
}