#include <stdlib.h>
#include <context_switch.h>
#include <proc.h>
#include <syscall.h>
#include <simics.h>
#include <scheduler.h>
#include <asm.h>

linklist_t scheduler_queue;

/**
 * @brief Initialize the scheduler
 *
 * @return 0 on success or a negative integer error code on failure
 */
int scheduler_init()
{
    if (linklist_init(&scheduler_queue) < 0) {
        return -1;
    }

    return 0;
}

/**
 * @brief The scheduler's handler to timer interrupts.
 *
 * @param num_ticks The number of ticks since the kernel began running.
 */
void scheduler_tick(unsigned num_ticks)
{
    disable_interrupts();

    int tid;
    linklist_remove_head(&scheduler_queue, (void**)&tid);
    linklist_add_tail(&scheduler_queue, (void*)tid);

    context_switch(tid);

    enable_interrupts();
}

/**
 * @brief Switches contexts to another thread.
 *
 *  Doesn't return until context switched back to.
 *
 * @param tid The tid of the thread to switch to.
 * @return Returns 0 on success or a negative error code on failure.
 */
int context_switch(int tid)
{
    if (tid == gettid()) {
        return 0;
    }

    tcb_t *new_tcb;
    if (hashtable_get(&tcbs, tid, (void**)&new_tcb) < 0) {
        return -1;
    }

    tcb_t *old_tcb;
    hashtable_get(&tcbs, gettid(), (void**)&old_tcb);

    if (store_regs(&old_tcb->regs)) {
        cur_tid = new_tcb->tid;
        restore_regs(&new_tcb->regs);
    } else {
        enable_interrupts();
    }

    return 0;
}

// TODO Add atomicity with make runnable, use either a spinlock or disable interruptss

/**
 * @brief Deschedules the current thread.
 *
 * @param flag If this points to a nonzero number, deschedule has no effect.
 * @return 0 on success or a negative error code on failure.
 */
int deschedule(int *flag) {
    if (flag == NULL) {
        return -1;
    }

    if (*flag) {
        return 0;
    }

    linklist_remove(&scheduler_queue, (void *)gettid());

    yield(-1);

    return 0;
}

/**
 * @brief Makes a thread runnable.
 *
 * @param tid The tid of the thread to make runnable
 * @return 0 iff the tid is of a thread previously descheduled by a call to
 * deschedule.
 */

int make_runnable(int tid) {
    if (linklist_contains(&scheduler_queue, (void *)tid)) {
        return -1;
    }

    linklist_add_tail(&scheduler_queue, (void*)tid);

    return 0;
}

/**
 * @brief Yields to another thread.
 *
 * @param tid The tid of the thread to yield to.
 * @return 0 on success or a negative error code on failure.
 */
int yield(int tid) {
    if (tid == -1) {
        linklist_remove_head(&scheduler_queue, (void**)&tid);
    } else {
        if (linklist_remove(&scheduler_queue, (void *)tid) < 0) {
            return -1;
        }
    }

    linklist_add_tail(&scheduler_queue, (void*)tid);

    context_switch(tid);

    return 0;
}
