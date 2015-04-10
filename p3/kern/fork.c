/** @file fork.c
 *  @brief Implements fork
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
#include <stdlib.h>
#include <x86/pic.h>
#include <exception.h>

int fork()
{
    tcb_t *old_tcb = gettcb();
    pcb_t *old_pcb = getpcb();

    if (old_pcb->num_threads > 1) //reject if multithreaded
        return -1;

    disable_interrupts();

    tcb_t *new_tcb;
    pcb_t *new_pcb;
    int new_tid;
    if ( (new_tid = proc_new_process(&new_pcb, &new_tcb)) < 0) {
        return -2;
    }

    old_pcb->num_children++;
    linklist_add_head(&old_pcb->children, new_pcb);
    new_pcb->parent_pcb = getpcb();

    //swap esp0's for old and new threads
    int cur_esp0 = old_tcb->esp0;
    old_tcb->esp0 = new_tcb->esp0;
    new_tcb->esp0 = cur_esp0;

    dup_swexn_handler(old_tcb, new_tcb);

    if (store_regs(&old_tcb->regs, cur_esp0)) { //new thread
        if (vm_copy(&new_pcb->pd) < 0) {
            // TODO handle bad things
        }

        set_cr3((unsigned)new_pcb->pd);
        cur_tcb = new_tcb;

        //give the old thread back his stack that the new one stole
        memcpy((void *)(old_tcb->esp0 - KERNEL_STACK_SIZE),
               (void *)(new_tcb->esp0 - KERNEL_STACK_SIZE), KERNEL_STACK_SIZE);

        linklist_add_head(&scheduler_queue, (void *)new_tcb);
        return 0;
    } else { //old thread
        // FIXME not always from timer call (see yield)
        pic_acknowledge_any_master();
        return new_tid;
    }
}

int thread_fork()
{
    tcb_t *old_tcb = gettcb();
    pcb_t *pcb = getpcb();

    disable_interrupts();

    tcb_t *new_tcb;

    if (proc_new_thread(pcb, &new_tcb) < 0) {
        return -1;
    }

    int new_tid = new_tcb->tid;

    //swap esp0's for old and new threads
    unsigned cur_esp0 = old_tcb->esp0;
    old_tcb->esp0 = new_tcb->esp0;
    new_tcb->esp0 = cur_esp0;

    if (store_regs(&old_tcb->regs, cur_esp0)) { //new_thread
        cur_tcb = new_tcb;

        //give the old thread back his stack that the new one stole
        memcpy((void *)(old_tcb->esp0 - KERNEL_STACK_SIZE),
               (void *)(new_tcb->esp0 - KERNEL_STACK_SIZE), KERNEL_STACK_SIZE);

        linklist_add_head(&scheduler_queue, (void *)new_tcb);

        enable_interrupts();
        return 0;
    } else { //old_thread
        // FIXME not always from timer call (see yield)
        pic_acknowledge_any_master();
        enable_interrupts();
        return new_tid;
    }
}
