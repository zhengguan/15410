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
#include <simics.h>
#include <asm.h>
#include <vm.h>

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

    if (cond_init(&pcb->waiter_cv) < 0) {
        return -3;
    }

    if (mutex_init(&pcb->vanished_task_mutex) < 0) {
        return -4;
    }

    if (linklist_init(&pcb->vanished_tasks) < 0) {
        return -5;
    }


    pcb->num_children = 0;
    pcb->pid = next_pid++;
    pcb->status = 0;

    int tid = proc_new_thread(pcb, tcb_out);
    if (tid < 0) {
        free(pcb);
        return tid;
    }

    pcb->first_tid = tid;
    pcb->num_threads = 1;

    hashtable_add(&pcbs, pcb->pid, (void *)pcb);

    if (pcb_out != NULL)
        *pcb_out = pcb;

    return tid;
}

/** @brief Creates a new TCB for a thread.
 *
 *  @return The tid on success, negative error code otherwise.
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
    tcb->esp0 = (unsigned)esp0;

    if (tcb_out != NULL)
        *tcb_out = tcb;


    hashtable_add(&tcbs, tcb->tid, (void*)tcb);
    linklist_add_tail(&pcb->threads, tcb);

    pcb->num_threads++;

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

    pcb_t *pcb;
    hashtable_get(&pcbs, tcb->pid, (void **)&pcb);

    pcb->status = status;
}

void vanish()
{

    tcb_t *tcb;
    hashtable_get(&tcbs, gettid(), (void **)&tcb);

    pcb_t *pcb;
    hashtable_get(&pcbs, tcb->pid, (void**)&pcb);

    disable_interrupts();

    if (pcb->num_threads == 1) {
        if (pcb->parent_pid < 0) {
            lprintf("tried to remove last thread of root process");
            MAGIC_BREAK;
        }

        pcb_t *parent_pcb;
        if (hashtable_get(&pcbs, pcb->parent_pid, (void*)&parent_pcb) < 0) {
            //TODO: add self to idle's vanished tasks; parent has already died
        } else {
            linklist_add_tail(&parent_pcb->vanished_tasks, pcb);
            cond_signal(&parent_pcb->waiter_cv);
        }

        pcb_t *dead_pcb;
        while (linklist_remove_head(&pcb->vanished_tasks, (void**)&dead_pcb) == 0)
            free(dead_pcb);
    }


    pcb->num_threads--;
    linklist_remove(&pcb->threads, (void*)gettid());
    linklist_remove(&scheduler_queue, (void*)gettid());

    enable_interrupts();

    if (yield(-1) < 0) {
        //TODO: what to do here. last thread
        while(1);
    }

    //to forgo compiler message
    while(1);
}

int wait(int *status_ptr)
{
    if (status_ptr && !vm_is_present_len(status_ptr, sizeof(int)))
        return -2;

    tcb_t *tcb;
    hashtable_get(&tcbs, gettid(), (void **)&tcb);

    pcb_t *pcb;
    hashtable_get(&pcbs, tcb->pid, (void**)&pcb);

    mutex_lock(&pcb->vanished_task_mutex);

    //would block forever in this case
    if (pcb->num_threads == 1 && pcb->num_children == 0) {
        return -1;
    }
    while (linklist_empty(&pcb->vanished_tasks))
        cond_wait(&pcb->waiter_cv, &pcb->vanished_task_mutex);

    pcb_t *dead_pcb;
    linklist_remove_head(&pcb->vanished_tasks, (void*)&dead_pcb);

    pcb->num_children--;
    mutex_unlock(&pcb->vanished_task_mutex);

    if (status_ptr) {
        *status_ptr = dead_pcb->status;
    }

    int ret = dead_pcb->first_tid;

    free(dead_pcb);

    return ret;
}
