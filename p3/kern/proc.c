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
#include <assert.h>

#define FIRST_TID 1

unsigned next_tid = FIRST_TID;

linklist_t tcbs_to_reap;

hashtable_t pcbs;
hashtable_t tcbs;

int cur_tid;
int idle_tid;
int init_tid;

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
    pcb->status = 0;
    pcb->pid = -1;

    int tid = proc_new_thread(pcb, tcb_out);
    if (tid < 0) {
        free(pcb);
        return tid;
    }

    pcb->pid = tid;
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
    if (pcb->pid == -1)
        pcb->pid = tcb->tid;
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

int reap_pcb(pcb_t *pcb, int *status_ptr)
{
    if (status_ptr)
        *status_ptr = pcb->status;
    int pid = pcb->pid;
    free(pcb);
    return pid;
}

void reap_tcb(tcb_t *tcb)
{
    free((void*)(tcb->esp0 - KERNEL_STACK_SIZE));
    free(tcb);
}

void reap_tcbs()
{
    tcb_t *tcb;
    while (linklist_remove_head(&tcbs_to_reap, (void**)&tcb) == 0)
        reap_tcb(tcb);
}

//After context switch, cannot come back
void destroy_thread(tcb_t *tcb) NORETURN;

void destroy_thread(tcb_t *tcb)
{
    linklist_add_tail(&tcbs_to_reap, tcb);
    int flag = 0;
    deschedule_kern(&flag, false);

    //FIXME: make compiler happy another way
    while (1);
}

void vanish()
{
    tcb_t *tcb;
    hashtable_get(&tcbs, gettid(), (void **)&tcb);
    hashtable_remove(&tcbs, gettid());

    pcb_t *pcb;
    hashtable_get(&pcbs, tcb->pid, (void**)&pcb);
    hashtable_remove(&pcbs, tcb->pid);


    disable_interrupts();

    if (pcb->num_threads == 1) {
        pcb_t *init_pcb;
        hashtable_get(&pcbs, init_tid, (void**)&init_pcb);

        assert(pcb->parent_pid >= FIRST_TID);

        pcb_t *parent_pcb;
        if (hashtable_get(&pcbs, pcb->parent_pid, (void*)&parent_pcb) < 0) {
            linklist_add_tail(&init_pcb->vanished_tasks, pcb);
            cond_signal(&init_pcb->waiter_cv);
        } else {
            linklist_add_tail(&parent_pcb->vanished_tasks, pcb);
            parent_pcb->num_children--;
            cond_signal(&parent_pcb->waiter_cv);
        }


        pcb_t *dead_pcb;
        while (linklist_remove_head(&pcb->vanished_tasks, (void**)&dead_pcb) == 0) {
            linklist_add_tail(&init_pcb->vanished_tasks, dead_pcb);
            cond_signal(&init_pcb->waiter_cv);
        }
    }

    pcb->num_threads--;
    linklist_remove(&pcb->threads, (void*)gettid());

    disable_interrupts();
    destroy_thread(tcb);
    panic("Failed to vanish.");
}

int wait(int *status_ptr)
{
    if (status_ptr && !vm_is_present_len(status_ptr, sizeof(int)))
        return -2;

    tcb_t *tcb;
    if (hashtable_get(&tcbs, gettid(), (void **)&tcb) < 0)
        lprintf("fail 1");
    pcb_t *pcb;
    if (hashtable_get(&pcbs, tcb->pid, (void**)&pcb) < 0)
        lprintf("fail 2");

    mutex_lock(&pcb->vanished_task_mutex);

    while (linklist_empty(&pcb->vanished_tasks)) {
        //would block forever in this case
        if (pcb->num_threads == 1 && pcb->num_children == 0) {
            return -1;
        }
        cond_wait(&pcb->waiter_cv, &pcb->vanished_task_mutex);
    }

    pcb_t *dead_pcb;
    linklist_remove_head(&pcb->vanished_tasks, (void*)&dead_pcb);

    mutex_unlock(&pcb->vanished_task_mutex);

    return reap_pcb(dead_pcb, status_ptr);
}

