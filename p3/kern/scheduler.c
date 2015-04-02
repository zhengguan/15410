#include <stdlib.h>
#include <context_switch.h>
#include <proc.h>
#include <syscall.h>
#include <simics.h>
#include <scheduler.h>
#include <asm.h>
#include <cr.h>
#include <driver.h>

linklist_t scheduler_queue;

linklist_t sleep_queue;

typedef struct sleep_info {
    int tid;
    unsigned wake_ticks;
} sleep_info_t;

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
 *  @param ticks The number of ticks since the kernel began running.
 *  @return Void.
 */
void scheduler_tick(unsigned ticks)
{
    // TODO add linklist_get method to use instead   
    // Wake sleeping threads
    sleep_info_t *sleep_info;
    while (linklist_remove_head(&sleep_queue, (void **)&sleep_info) == 0) {
        if (sleep_info->wake_ticks >= ticks) {
            linklist_add_head(&sleep_queue, (void *)sleep_info);
            break;
        }
        make_runnable(sleep_info->tid);
    }
    
    int tid;
    linklist_remove_head(&scheduler_queue, (void**)&tid);
    linklist_add_tail(&scheduler_queue, (void*)tid);

    context_switch(tid);
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

    if (store_regs(&old_tcb->regs, old_tcb->esp0)) {
        cur_tid = new_tcb->tid;
        set_esp0(new_tcb->esp0);
        restore_regs(&new_tcb->regs, new_tcb->esp0);
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
        if (linklist_remove_head(&scheduler_queue, (void**)&tid) < 0) {
            return -1;
        }
    } else {
        if (linklist_remove(&scheduler_queue, (void *)tid) < 0) {
            return -2;
        }
    }

    linklist_add_tail(&scheduler_queue, (void*)tid);

    if (context_switch(tid) < 0) {
        return -3;
    }
    
    notify_interrupt_complete(); //we are coming from timer call but not returning

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
    // TODO fail id last thread in scheduler queue?
    
    if (flag == NULL) {
        return -1;
    }

    if (*flag) {
        return 0;
    }

    linklist_remove(&scheduler_queue, (void *)gettid());

    if (yield(-1) < 0) {
        return -2;
    }

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

/** @brief Deschedules the calling thread until atleast ticks timer interrupts
 *  have occured after the call.
 *
 *  Returns immediately if ticks is zero.  Returns an integer error code less
 *  than zero if ticks is negative.  Returns zero otherwise.
 *
 *  @param ticks The number of timer interrupts the thread is descheduled.
 *  @return 0 on success, negative error code otherwise.
 */
int sleep(int ticks)
{
    if (ticks < 0) {
        return -1;
    }
    
    if (ticks == 0) {
        return 0;
    }
    
    sleep_info_t sleep_info;
    sleep_info.tid = gettid();
    sleep_info.wake_ticks = get_ticks() + ticks;
    
    linklist_add_head(&sleep_queue, (void *)&sleep_info);
    
    int flag = 0;
    if (deschedule(&flag) < 0) {
        return -2;
    }
    
    return 0;
}
