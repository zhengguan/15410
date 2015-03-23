/** @file proc.c
 *  @brief Manages kernel processes and threads.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <proc.h>
#include <cr.h>
#include <syscall.h>
#include <stdlib.h>
#include <linklist.h>
#include <hashtable.h>
#include <scheduler.h>

int next_pid = 1;
int next_tid = 1;

hashtable_t pcbs;
hashtable_t tcbs;

// TODO get rid of cur_tid, use scheduler queue instead
int cur_pid;
int cur_tid;

/** @brief Initializes the PCB and TCB data structures.
 *
 *  @return 0 on success, negative error code otherwise.
 */
int proc_init() {
     if (hashtable_init(&pcbs, PCB_HT_SIZE) < 0) {
        return -1;
     }
     
     if (hashtable_init(&tcbs, TCB_HT_SIZE)) {
        return -2;
     }

     return 0;
}

/** @brief Creates a new PCB for a process.
 *
 *  @return The thread ID of the newly created thread on success, negative
 *  error code otherwise.
 */
int proc_new_process() {
    pcb_t *pcb = malloc(sizeof(pcb_t));
    if (pcb == NULL) {
        return -1;
    }

    if (linklist_init(&pcb->threads) < 0) {
        return -2;
    }

    pcb->pid = next_pid++;
    cur_pid = pcb->pid;

    hashtable_add(&pcbs, pcb->pid, (void *)pcb);

    return proc_new_thread();
}

/** @brief Creates a new TCB for a thread.
 *
 *  @return The thread ID of the newly created thread on success, negative
 *  error code otherwise.
 */
int proc_new_thread() {
    // TODO may need to disable interrupts to prevent context switching in here

    tcb_t *tcb = malloc(sizeof(tcb_t));
    if (tcb == NULL) {
        return -3;
    }

    tcb->tid = next_tid++;
    tcb->pid = cur_pid;
    cur_tid = tcb->tid;
    
    set_esp0((unsigned)malloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE);

    pcb_t *pcb;
    if (hashtable_get(&pcbs, tcb->pid, (void**)&pcb) < 0) {
        return -4;
    }

    hashtable_add(&tcbs, tcb->tid, (void*)tcb);
    linklist_add_tail(&pcb->threads, tcb);

    linklist_add_tail(&scheduler_queue, (void *)tcb->tid);

    return tcb->tid;
}

/** @brief Returns the thread ID of the invoking thread.
 *
 *  @return The thread ID of the invoking thread.
 */
int gettid()
{
    return cur_tid;
}

