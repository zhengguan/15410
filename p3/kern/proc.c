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
#include <asm.h>
#include <vm.h>
#include <assert.h>
#include <stdio.h>
#include <stdarg.h>
#include <rwlock.h>
#include <exception.h>
#include <malloc_wrappers.h>
#include <asm_common.h>


#define TCB_HT_SIZE 128
#define MEMLOCK_SIZE 128
#define ALLOC_PAGES_HASHTABLE_SIZE 128

#define FIRST_TID 1

rwlock_t tcbs_lock;
hashtable_t tcbs;

unsigned next_tid = FIRST_TID;

tcb_t *cur_tcb = NULL;
tcb_t *idle_tcb;
pcb_t *init_pcb;

mutex_t thread_reap_mutex;
cond_t thread_reap_cv;
linklist_t tcbs_to_reap;

/**
 * @brief Tests whether two pointers are the same.
 *
 * @param tcb0 The first pointer.
 * @param tcb1 The second pointer.
 *
 * @return True if tcb0 == tcb1. False otherwise.
 */
static bool ident(void *tcb0, void *tcb1)
{
    return tcb0 == tcb1;
}

/** @brief Initialize the process mutexes for given process mutex struct.
 *
 *  @param locks The process mutex stuct.
 *  @return 0 on success, negative error code otherwise.
 */
static int init_locks(locks_t *locks) {
    // Initialize memlock last to prevent memory leaks from hashtable_init
    if ((mutex_init(&locks->vm_lock) < 0) ||
        (mutex_init(&locks->alloc_pages_lock) < 0) ||
        (memlock_init(&locks->memlock, MEMLOCK_SIZE) < 0)) {
        return -1;
    }

    return 0;
}

/** @brief Initializes the PCB and TCB data structures.
 *
 *  @return 0 on success, negative error code otherwise.
 */
int proc_init() {
     if (hashtable_init(&tcbs, TCB_HT_SIZE) < 0) {
        return -2;
     }

     if (linklist_init(&tcbs_to_reap) < 0) {
        return -3;
     }

     if (mutex_init(&thread_reap_mutex) < 0) {
        return -4;
     }

     if (cond_init(&thread_reap_cv) < 0) {
        return -5;
     }

     if (rwlock_init(&tcbs_lock) < 0) {
        return -7;
     }

     return 0;
}

/** @brief Creates a new PCB for a process.
 *
 *  @param pcb_out Memory to store the newly created PCB.
 *  @param tcb_out Memory to store the newly created TCB.
 *
 *  @return The thread ID of the newly created thread on success, negative
 *  error code otherwise.
 */
int proc_new_process(pcb_t **pcb_out, tcb_t **tcb_out) {
    pcb_t *pcb = malloc(sizeof(pcb_t));
    if (pcb == NULL) {
        return -1;
    }

    if (linklist_init(&pcb->children) < 0) {
        return -2;
    }

    if (cond_init(&pcb->wait_cv) < 0) {
        return -3;
    }

    if (mutex_init(&pcb->proc_mutex) < 0) {
        return -4;
    }

    if (linklist_init(&pcb->vanished_procs) < 0) {
        return -5;
    }

    if (hashtable_init(&pcb->alloc_pages, ALLOC_PAGES_HASHTABLE_SIZE) < 0) {
        return -5;
    }

    if (init_locks(&pcb->locks) < 0) {
        return -6;
    }

    pcb->pid = -1;
    pcb->status = 0;
    pcb->num_threads = 0;
    pcb->num_children = 0;
    pcb->pd = NULL;

    int tid = proc_new_thread(pcb, tcb_out);
    if (tid < 0) {
        free(pcb);
        return -7;
    }

    if (pcb_out != NULL) {
        *pcb_out = pcb;
    }

    return tid;
}

/** @brief Creates a new TCB for a thread.
 *
 *  @param pcb The PCB of the process in which to create the thread.
 *  @param tcb_out Memory to store the newly created TCB.
 *  @return The therad ID of the newly created thread on success, negative
 *  eror code otherwise.
 */
int proc_new_thread(pcb_t *pcb, tcb_t **tcb_out) {
    tcb_t *tcb = malloc(sizeof(tcb_t));
    if (tcb == NULL) {
        return -1;
    }

    char *esp0 = malloc(KERNEL_STACK_SIZE) + KERNEL_STACK_SIZE;
    if (esp0 == NULL) {
        free(tcb);
        return -2;
    }

    tcb->tid = next_tid++;
    if (pcb->pid == -1) {
        pcb->pid = tcb->tid;
    }

    tcb->pcb = pcb;
    tcb->esp0 = (unsigned)esp0;
    tcb->sleep_flag = 0;
    deregister_swexn_handler(tcb);

    pcb->num_threads++;

    if (tcb_out != NULL) {
        *tcb_out = tcb;
    }

    rwlock_lock(&tcbs_lock, RWLOCK_WRITE);
    int hashadd = hashtable_add(&tcbs, tcb->tid, (void*)tcb);
    rwlock_unlock(&tcbs_lock);

    if (hashadd < 0) {
        free(tcb);
        free(esp0);
        return -3;
    }

    return tcb->tid;
}


/**
 * @brief Gets the current tcb.
 * @return The tcb.
 */
tcb_t *gettcb()
{
    return cur_tcb;
}

/** @brief Returns the thread ID of the invoking thread.
 *
 *  @return The thread ID of the invoking thread or 0 if invoked before a
 *  thread is created.
 */
int gettid()
{
    return gettcb()->tid;
}

/**
 * @brief Gets the current pcb.
 * @return The pcb.
 */
pcb_t *getpcb()
{
    return gettcb()->pcb;
}

/** @brief Returns the process ID of the invoking thread.
 *
 *  @return The process ID of the invoking thread or 0 if invoked before a
 *  thread is created.
 */
int getpid()
{
    return getpcb()->pid;
}

/** @brief Looks up a tcb given a tid.
 *
 *  @param tid The tid.
 *  @return The tcb or NULL if it does not exist.
 */
tcb_t *lookup_tcb(int tid)
{
    tcb_t *tcb;
    rwlock_lock(&tcbs_lock, RWLOCK_READ);
    if (hashtable_get(&tcbs, tid, (void**)&tcb) < 0)
        tcb = NULL;
    rwlock_unlock(&tcbs_lock);
    return tcb;
}

/** @brief Sets the exit status of the current task to status.
 *
 *  @param status The status.
 *  @return Void.
 */
void set_status(int status)
{
    getpcb()->status = status;
}
/** @brief Reaps a process.
 *
 *  @param pcb The PCB of the process to reap.
 *  @param status_ptr A pointer to memory in which to store the
 *  status of the exited process.
 *
 *  @return The PID of the process to reap.
 */
int reap_pcb(pcb_t *pcb, int *status_ptr)
{
    assert(pcb->pid != getpid());
    int pid = pcb->pid;
    if (status_ptr) {
        *status_ptr = pcb->status;
    }

    if (pcb->pd) {
        vm_destroy(pcb->pd);
    }

    hashtable_destroy(&pcb->alloc_pages);
    memlock_destroy(&pcb->locks.memlock);
    free(pcb);
    return pid;
}

/** @brief Reaps a thread.
 *
 *  @param tcb The TCB of the thread to reap.
 *  @return Void.
 */
void reap_tcb(tcb_t *tcb)
{
    rwlock_lock(&tcbs_lock, RWLOCK_WRITE);
    hashtable_remove(&tcbs, tcb->tid, NULL);
    rwlock_unlock(&tcbs_lock);

    free((void*)(tcb->esp0 - KERNEL_STACK_SIZE));
    free(tcb);
}

/** @brief Reaping all threads as they vanish.
 *
 *  To run in its own process.  Does not return.
 *
 *  @return Does not return.
 */
void thread_reaper()
{
    mutex_lock(&thread_reap_mutex);
    while (1) {
        tcb_t *tcb;
        while (linklist_remove_head(&tcbs_to_reap, (void**)&tcb, NULL) < 0) {
            cond_wait(&thread_reap_cv, &thread_reap_mutex);
        }
        reap_tcb(tcb);
    }
}

/** @brief Destroys a thread.
 *
 *  Queues a thread for reaping, deschedules the thread, and context switches.
 *
 *  @param tcb The TCB of the thread to destroy.
 *  @return Does not return.
 */
static void destroy_thread(tcb_t *tcb) NORETURN;
static void destroy_thread(tcb_t *tcb)
{
    listnode_t node;
    linklist_add_tail(&tcbs_to_reap, tcb, &node);
    cond_signal(&thread_reap_cv);

    int flag = 0;
    deschedule_kern(&flag, false);

    panic("Running vanished thread");

    while(1);
}

/** @brief Terminates the execution of the calling thread.
 *
 *  If the invoking thread is the last thread in its task, the kernel
 *  deallocates all resources in use by the task and makes the exit status of
 *  the task available to the parent task.  If the parent task is no longer
 *  running, the exit status of the task is made available to the
 *  kernel-launched “init” task instead.
 *
 *  @return Does not return.
 */
void vanish()
{
    assert(interrupts_enabled());
    tcb_t *tcb = gettcb();
    pcb_t *pcb = getpcb();
    deregister_swexn_handler(gettcb());
    if (pcb->num_threads == 1) {
        vm_clear();
    }

    //Tell children their father died :(
    pcb_t *child;
    while (linklist_remove_head(&pcb->children, (void**)&child, NULL) == 0) {
        mutex_lock(&child->proc_mutex);
        child->parent_pcb = NULL;
        mutex_unlock(&child->proc_mutex);
    }

    /* Get the mutexes we need */
    mutex_lock(&pcb->proc_mutex);
    if (pcb->num_threads == 1) {
        if (pcb->parent_pcb)
            mutex_lock(&pcb->parent_pcb->proc_mutex);
        mutex_lock(&init_pcb->proc_mutex);
    }
    mutex_lock(&thread_reap_mutex);
    mutex_lock(&malloc_mutex);

    /* Disable interupts and mutexes */
    disable_interrupts();

    /* Unlock the mutexes for when we vanish */
    mutex_unlock(&malloc_mutex);
    mutex_unlock(&thread_reap_mutex);
    if (pcb->num_threads == 1) {
        mutex_unlock(&init_pcb->proc_mutex);
        if (pcb->parent_pcb)
            mutex_unlock(&pcb->parent_pcb->proc_mutex);
    }
    mutex_unlock(&pcb->proc_mutex);

    // No need for locking because interrupts are disabled
    if (pcb->num_threads == 1) {
        if (pcb->parent_pcb) {
            linklist_remove(&pcb->parent_pcb->children, pcb, ident, NULL, NULL);
            linklist_add_tail(&pcb->parent_pcb->vanished_procs, pcb, &pcb->pcb_listnode);
            pcb->parent_pcb->num_children--;
            cond_signal(&pcb->parent_pcb->wait_cv);
        } else {
            linklist_add_tail(&init_pcb->vanished_procs, pcb, &pcb->pcb_listnode);
            cond_signal(&init_pcb->wait_cv);
        }

        //Pass dead children to init
        pcb_t *dead_pcb;
        listnode_t *node;
        while (linklist_remove_head(&pcb->vanished_procs, (void**)&dead_pcb, &node) == 0) {
            linklist_add_tail(&init_pcb->vanished_procs, dead_pcb, node);
            cond_signal(&init_pcb->wait_cv);
        }
    }

    pcb->num_threads--;

    destroy_thread(tcb);
}

/** @brief Collects the exit status of a child task and stores it in the
 *  integer referenced by status_ptr.
 *
 *  Threads which cannot collect an already-exited child task when there exist
 *  child tasks which have not yet exited will block until a child task exits
 *  and collect the status of an exited child task. However, threads which
 *  will definitely not be able to collect the status of an exited child task
 *  in the future will return an integer error code less than zero.

 *  @status_ptr Memory to store the exit status.
 *  @return The thread ID of the original thread of the exiting task,
 *  negative error code otherwise.
 */
int wait(int *status_ptr)
{
    pcb_t *pcb = getpcb();

    mutex_lock(&pcb->proc_mutex);

    pcb_t *dead_pcb;
    while (linklist_remove_head(&pcb->vanished_procs, (void*)&dead_pcb, NULL) < 0) {
        // Would block forever in this case
        if (pcb->num_threads == 1 && pcb->num_children == 0) {
            mutex_unlock(&pcb->proc_mutex);
            return -1;
        }
        cond_wait(&pcb->wait_cv, &pcb->proc_mutex);
    }

    mutex_unlock(&pcb->proc_mutex);
    return reap_pcb(dead_pcb, status_ptr);
}

/**
 * @brief Kills a user thread.
 * @details Prints an error message.
 *
 * @param fmt The format of the string to print.
 */
void proc_kill_thread(const char *fmt, ...)
{
    // Print error msg
    va_list vl;
    char buf[80];

    va_start(vl, fmt);
    vsnprintf(buf, sizeof (buf), fmt, vl);
    va_end(vl);

    va_start(vl, fmt);
    vprintf(fmt, vl);
    va_end(vl);

    printf("\n");

    // TODO Register Dump

    /* Kill thread */
    pcb_t *pcb = getpcb();

    if (pcb->num_threads == 1) {
        set_status(-2);
    }
    vanish();
}