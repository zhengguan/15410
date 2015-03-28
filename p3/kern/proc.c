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

unsigned next_pid = 1;
unsigned next_tid = 1;

hashtable_t pcbs;
hashtable_t tcbs;

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
int proc_new_process(pcb_t **pcb_out, tcb_t **tcb_out) {
    pcb_t *pcb = malloc(sizeof(pcb_t));
    if (pcb == NULL) {
        return -1;
    }

    if (linklist_init(&pcb->threads) < 0) {
        return -2;
    }

    pcb->pid = next_pid++;


    int err = proc_new_thread(pcb, tcb_out);
    if (err < 0) {
        free(pcb);
        return err;
    }

    hashtable_add(&pcbs, pcb->pid, (void *)pcb);

    if (pcb_out != NULL)
        *pcb_out = pcb;

    return 0;
}

/** @brief Creates a new TCB for a thread.
 *
 *  @return The 0 success, negative error code otherwise.
 */
int proc_new_thread(pcb_t *pcb, tcb_t **tcb_out) {
    // TODO may need to disable interrupts to prevent context switching in here

    tcb_t *tcb = malloc(sizeof(tcb_t));
    if (tcb == NULL) {
        return -4;
    }

    char *esp0 = malloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE;
    if (esp0 == NULL) {
        free(tcb);
        return -5;
    }

    tcb->tid = next_tid++;
    // TODO this is broken
    tcb->pid = pcb->pid;
    tcb->status = 0;
    tcb->esp0 = (unsigned)esp0;

    if (tcb_out != NULL)
        *tcb_out = tcb;


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

int getpid()
{
    tcb_t *tcb;
    hashtable_get(&tcbs, gettid(), (void**)&tcb);
    return tcb->pid;
}

void set_status(int status)
{
    tcb_t *tcb;
    hashtable_get(&tcbs, gettid(), (void **)&tcb);
    tcb->status = status;
}
