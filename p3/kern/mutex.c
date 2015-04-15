/** @file mutex.c
 *  @brief This file implements mutexes.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <mutex.h>
#include <stdlib.h>
#include <syscall.h>
#include <linklist.h>
#include <waiter.h>
#include <scheduler.h>
#include <proc.h>
#include <assert.h>
#include <asm_common.h>

/** @brief Initializes a mutex.
 *
 *  @param mp The mutex.
 *  @return 0 on success, negative error code otherwise.
 */
int mutex_init(mutex_t *mp)
{
    if (mp == NULL) {
        return -1;
    }

    if (spinlock_init(&mp->wait_lock) < 0) {
        return -2;
    }

    if (linklist_init(&mp->wait_list) < 0){
        return -3;
    }

    mp->count = 1;
    mp->tid = -1;

    return 0;
}

/** @brief Locks a mutex.  Blocks until mutex is acquired.
 *
 *  @param mp The mutex.
 *  @return Void.
 */
void mutex_lock(mutex_t *mp)
{
    if (kernel_init) {
        return;
    }

    waiter_t waiter = {gettcb(), 0};
    listnode_t node;

    spinlock_lock(&mp->wait_lock);
    /* Dont yet have lock */
    assert(mp != NULL);
    assert(mp->tid != gettid());
    if (mp->count <= 0) {
        linklist_add_tail(&mp->wait_list, (void*)&waiter, &node);
    } else {
        /* Immedietly got lock */
        waiter.reject = 1;
    }
    mp->count--;
    spinlock_unlock(&mp->wait_lock);

    deschedule_kern(&waiter.reject, false);

    /* Have lock */
    spinlock_lock(&mp->wait_lock);
    mp->tid = gettid();
    spinlock_unlock(&mp->wait_lock);

}

/** @brief Unlocks a mutex.
 *
 *  @param mp The mutex.
 *  @return Void.
 */
void mutex_unlock(mutex_t *mp)
{
    if (kernel_init) {
        return;
    }

    spinlock_lock(&mp->wait_lock);
    /* Have Lock */
    assert(mp != NULL);
    assert(mp->count <= 0);
    assert(mp->tid == gettid());
    if (mp->count < 0) {
        waiter_t *waiter;
        linklist_remove_head(&mp->wait_list, (void**)&waiter, NULL);
        waiter->reject = 1;
        make_runnable_kern(waiter->tcb, false);
    }
    mp->tid = -1;
    mp->count++;
    spinlock_unlock(&mp->wait_lock);
}

