/** @file proc.c
 *  @brief Manages kernel processes and threads.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <proc.h>
#include <syscall.h>
#include <stdlib.h>
#include <linklist.h>
#include <hashtable.h>

int next_pid = 1;
int next_tid = 1;

hashtable_t pcbs;
hashtable_t tcbs;

int cur_pid;
int cur_tid;

/** @brief Initializes the PCB and TCB data structures.
 *
 *  @return 0 on success, negative error code otherwise.
 */
int proc_init() {
     hashtable_init(&pcbs, PCB_HT_SIZE);
     hashtable_init(&tcbs, TCB_HT_SIZE);
     
     return 0;
}

/** @brief Creates a new PCB for a process.
 *
 *  @return 0 on success, negative error code otherwise.
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
    
    proc_new_thread();

    return 0;
}

/** @brief Creates a new TCB for a thread.
 *
 *  @return 0 on success, negative error code otherwise.
 */
int proc_new_thread() {
    tcb_t *tcb = malloc(sizeof(tcb_t));
    if (tcb == NULL) {
        return -1;
    }

    tcb->tid = next_tid++;
    tcb->pid = cur_pid;
    cur_tid = tcb->tid;

    pcb_t *pcb;
    if (!hashtable_get(&pcbs, tcb->pid, (void**)&pcb)) {
        return -2;
    }

    hashtable_add(&tcbs, tcb->tid, (void*)tcb);
    linklist_add_tail(&pcb->threads, tcb);

    return 0;
}

/** @brief Returns the thread ID of the invoking thread.
 *
 *  @return The thread ID of the invoking thread.
 */
int gettid()
{
    return cur_tid;
}

