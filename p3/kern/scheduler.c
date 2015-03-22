#include <stdlib.h>
#include <context_switch.h>
#include <proc.h>
#include <syscall.h>
#include <simics.h>
#include <scheduler.h>
#include <asm.h>

linklist_t scheduler_queue;

/** @brief Initializes the scheduler.
 *
 *  @return 0 on success, negative error code otherwise.
 */
int scheduler_init()
{
    if (linklist_init(&scheduler_queue) < 0) {
        return -1;
    }

    return 0;
}

/** @brief Scheduler timer interrupt handler.
 *
 *  @param num_ticks The number of ticks since the kernel began running.
 *  @return Void.
 */
void scheduler_tick(unsigned num_ticks)
{
    lprintf("Begin tick");    
    
    int tid;
    linklist_remove_head(&scheduler_queue, (void**)&tid);
    linklist_add_tail(&scheduler_queue, (void*)tid);

    context_switch(tid);
    
    lprintf("End tick");    
}

/** @brief Context switches to the thread with ID tid.
 *
 *  Doesn't return until context switched back to.
 *
 *  @param tid The tid of the thread to context switch to.
 *  @return 0 on success, negative error code otherwise.
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

    disable_interrupts();
    if (store_regs(&old_tcb->regs)) {
        cur_tid = new_tcb->tid;
        restore_regs(&new_tcb->regs);
    }
    enable_interrupts();

    return 0;
}

/** @brief Defers execution of the invoking thread to a time determined by the
 *  scheduler.
 *
 *  If tid is -1 the scheduler will determine which thread to run next.
 *
 *  @param tid The tid of the thread to yield to.
 *  @return 0 on success, negative error code otherwise.
 */
int yield(int tid)
{
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

// TODO Add atomicity with make runnable, use either a spinlock or disable interruptss

/** @brief Deschedules the calling thread.
 *
 *  If the integer pointed to by reject is non-zero, the call returns
 *  immediately returns with return value 0.  If the integer pointed to by
 *  reject is zero, then the calling thread will not be run by the scheduler
 *  until a make_runnable() call is made specifying the descheduled thread, at
 *  point deschedule() will return zero.
 *
 *  @param flag If this points to a nonzero integer, deschedule returns
 *  immediately.
 *  @return 0 on success, negative error code otherwise.
 */
int deschedule(int *flag)
{
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

/** @brief Makes the descheduled thread with ID tid runnable.
 *
 *  @param tid The tid of the thread to make runnable.
 *  @return 0 on success, negative error code otherwise.
 */

int make_runnable(int tid)
{
    if (linklist_contains(&scheduler_queue, (void *)tid)) {
        return -1;
    }

    linklist_add_tail(&scheduler_queue, (void*)tid);

    return 0;
}
