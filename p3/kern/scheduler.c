/** @file scheduler.c
 *  @brief Manages the scheduler and context switching.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <stdlib.h>
#include <context_switch.h>
#include <proc.h>
#include <syscall.h>
#include <simics.h>
#include <scheduler.h>
#include <asm.h>
#include <cr.h>
#include <x86/pic.h>
#include <common_kern.h>
#include <vm.h>
#include <assert.h>
#include <asm_common.h>

linklist_t scheduler_queue;
linklist_t sleep_queue;

typedef struct sleep_info {
    tcb_t *tcb;
    unsigned wake_ticks;
} sleep_info_t;

/**
 * @brief Determines whether a tcb has a tid.
 *
 * @param tcb The tcb.
 * @param tid The tid.
 *
 * @return True if the tcb has the tid. False otherwise.
 */
static bool tcb_is_tid(void *tcb, void *tid)
{
    return ((tcb_t *)tcb)->tid == (int)tid;
}

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

/** @brief Compares the wake times of sleep_info structs.  See linklist.h.
 *
 *  @param info1 First sleep_info_t.
 *  @param info2 Second sleep_info_t.
 *  @return A negative value of info1 should wake before info2, a positive
 *  value if info2 should wake before info1, 0 if they should wake at the same
 *  time.
 */
static int sleep_info_cmp(void *info1, void *info2) {
    return ((sleep_info_t *)info1)->wake_ticks -
           ((sleep_info_t *)info2)->wake_ticks;
}

/** @brief Initializes the scheduler.
 *
 *  @return 0 on success, negative error code otherwise.
 */
int scheduler_init()
{
    if (linklist_init(&scheduler_queue) < 0) {
        return -1;
    }
    if (linklist_init(&sleep_queue) < 0) {
        return -2;
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
    disable_interrupts();
    // Wake sleeping threads
    sleep_info_t *sleep_info;
    //FIXME: ta's dislike unbounded times in handlers. maybe max to awaken at a time?
    while ((linklist_peek_head(&sleep_queue, (void **)&sleep_info) == 0) &&
                                        sleep_info->wake_ticks <= ticks) {
        sleep_info->tcb->sleep_flag = 1;
        make_runnable_kern(sleep_info->tcb, false);
        assert (linklist_remove_head(&sleep_queue, NULL, NULL) == 0);
    }

    tcb_t *tcb;
    if (linklist_rotate_head(&scheduler_queue, (void**)&tcb) < 0) {
        assert(gettid() == idle_tcb->tid);
        return;
    }
    assert((unsigned)tcb < USER_MEM_START);
    // //lprintf("scheduler tick got %p", tcb);
    assert(context_switch(tcb) == 0);
}

/** @brief Context switches to the thread with ID tid.
 *
 *  Doesn't return until context switched back to.
 *
 *  @param tid The tid of the thread to context switch to.
 *  @return 0 on success, negative error code otherwise.
 */
int context_switch(tcb_t *new_tcb)
{
    if (new_tcb == NULL)
        return -1;

    tcb_t *old_tcb = gettcb();
    if (new_tcb->tid == old_tcb->tid) {
        return 0;
    }

    if (new_tcb->regs.eip == 0x28d2e8) {
        MAGIC_BREAK;
    }

    disable_interrupts();
    lprintf("New(%d): %p  Old(%d): %p", new_tcb->tid, (void *)new_tcb->regs.cr3, old_tcb->tid, (void *)old_tcb->regs.cr3);
    if (store_regs(&old_tcb->regs, old_tcb->esp0)) {
        cur_tcb = new_tcb;
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
    disable_interrupts();
    tcb_t *tcb;
    if (tid == -1) {
        if (linklist_rotate_head(&scheduler_queue, (void**)&tcb) < 0) {
            //no threads so run idle
            assert (context_switch(idle_tcb) == 0);
            pic_acknowledge_any_master();
            enable_interrupts();
            return 0;
        }
    } else if (linklist_rotate_val(&scheduler_queue, (void *)tid, tcb_is_tid, (void**)&tcb) < 0) {
        enable_interrupts();
        return -1;
    }

    assert((unsigned)tcb < USER_MEM_START);
    assert (context_switch(tcb) == 0);

    pic_acknowledge_any_master();
    return 0;
}

// TODO Add atomicity with make runnable, use either a spinlock or disable interrupts


/** @brief Deschedules the calling thread.
 *
 *  If the integer pointed to by reject is non-zero, the call returns
 *  immediately returns with return value 0. If the integer pointed to by
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
    return deschedule_kern(flag, true);
}

/** @brief Deschedules the calling thread.
 *
 *  If the integer pointed to by reject is non-zero, the call returns
 *  immediately returns with return value 0.  If the integer pointed to by
 *  reject is zero, then the calling thread will not be run by the scheduler
 *  until a make_runnable() call is made specifying the descheduled thread, at
 *  point deschedule() will return zero. The thread cannot be rescheduled
 *  by a call to make_runnable if user is false.
 *
 *  @param flag If this points to a nonzero integer, deschedule returns
 *  immediately.
 *  @param user User bool
 *  @return 0 on success, negative error code otherwise.
 */
int deschedule_kern(int *flag, bool user)
{
    if (flag == NULL)
        return -1;

    disable_interrupts();

    if (*flag) {
        enable_interrupts();
        return 0;
    }

    assert(linklist_remove(&scheduler_queue, (void*)gettcb(), ident, NULL, NULL) == 0);

    gettcb()->user_descheduled = user;

    if (yield(-1) < 0) {
        return -2;
    }

    enable_interrupts();
    return 0;
}

/** @brief Makes the descheduled thread with ID tid runnable if the thread
 *  was descheduled by a call to deschedule.
 *
 *  @param tid The tid of the thread to make runnable.
 *  @return 0 on success, negative error code otherwise.
 */
int make_runnable(int tid)
{
    tcb_t *tcb = lookup_tcb(tid);
    if (tcb == NULL)
        return -1;
    return make_runnable_kern(tcb, true);
}

/** @brief Makes the descheduled thread with ID tid runnable.
 *
 *  It is an error to try to reschedule a thread who was descheduled by
 *  deschedule_kern(tid, false) if user is true.
 *
 *  @param tid The tid of the thread to make runnable.
 *  @param user User bool
 *  @return 0 on success, negative error code otherwise.
 */
int make_runnable_kern(tcb_t *tcb, bool user)
{
    bool in = interrupts_enabled();
    disable_interrupts();
    if (linklist_contains(&scheduler_queue, (void *)tcb, ident)) {
        enable_interrupts();
        return -1;
    }

    if (user && !tcb->user_descheduled) {
        enable_interrupts();
        return -2;
    }
    assert((unsigned)tcb < USER_MEM_START);

    linklist_add_head(&scheduler_queue, (void*)tcb, &tcb->scheduler_listnode);
    if (in)
        enable_interrupts();

    return 0;
}

/** @brief Deschedules the calling thread until at least ticks timer interrupts
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
    sleep_info.tcb = gettcb();
    sleep_info.tcb->sleep_flag = 0;
    sleep_info.wake_ticks = get_ticks() + ticks;

    listnode_t node;

    disable_interrupts();
    linklist_add_sorted(&sleep_queue, (void *)&sleep_info, sleep_info_cmp, &node);
    enable_interrupts();

    if (deschedule_kern(&sleep_info.tcb->sleep_flag, false) < 0) {
        return -2;
    }


    return 0;
}
