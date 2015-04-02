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
#include <driver.h>

int fork()
{
    lprintf("fork");
    tcb_t *old_tcb;
    hashtable_get(&tcbs, gettid(), (void**)&old_tcb);

    disable_interrupts();

    tcb_t *new_tcb;

    if (proc_new_process(NULL, &new_tcb) < 0) {
        return -1;
    }

    //swap esp0's for old and new threads
    int cur_esp0 = old_tcb->esp0;
    old_tcb->esp0 = new_tcb->esp0;
    new_tcb->esp0 = cur_esp0;

    int new_tid = new_tcb->tid; //in case new thread dies before we return

    lprintf("fork storing %d, %p, %p", gettid(), (void*)cur_esp0, (void*)get_cr3());
    if (store_regs(&old_tcb->regs, cur_esp0)) { //new thread

        set_cr3((unsigned)vm_copy());

        cur_tid = new_tid;

        //give the old thread back his stack that the new one stole
        memcpy((void *)(old_tcb->esp0 - KERNEL_STACK_SIZE), (void *)(new_tcb->esp0 - KERNEL_STACK_SIZE), KERNEL_STACK_SIZE);


        linklist_add_tail(&scheduler_queue, (void *)new_tid);

        enable_interrupts();
        return 0;
    } else { //old thread
        notify_interrupt_complete(); //we are coming from timer call but not returning
        enable_interrupts();
        return new_tid;
    }
}

int thread_fork()
{
    tcb_t *old_tcb;
    hashtable_get(&tcbs, gettid(), (void**)&old_tcb);

    pcb_t *pcb;
    hashtable_get(&pcbs, old_tcb->pid, (void**)&pcb);

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
        cur_tid = new_tid;

        //give the old thread back his stack that the new one stole
        memcpy((void *)(old_tcb->esp0 - KERNEL_STACK_SIZE), (void *)(new_tcb->esp0 - KERNEL_STACK_SIZE), KERNEL_STACK_SIZE);

        linklist_add_tail(&scheduler_queue, (void *)new_tid);

        enable_interrupts();
        return 0;
    } else { //old_thread
        notify_interrupt_complete(); //we are returning from timer but didn't come from timer
        enable_interrupts();
        return new_tid;
    }
}