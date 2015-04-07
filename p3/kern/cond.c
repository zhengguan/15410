/** @file cond.c
 *  @brief This file implements the interface for condition variables.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <cond.h>
#include <stdlib.h>
#include <syscall.h>
#include <waiter.h>
#include <scheduler.h>

/** @brief Initializes a condition variable.
 *
 *  @param cv The condition variable.
 *  @return 0 on success, number error code otherwise.
 */
int cond_init(cond_t *cv)
{
    if (cv == NULL) {
        return -1;
    }

    if (spinlock_init(&cv->wait_lock) < 0) {
        return -2;
    }

    if (linklist_init(&cv->wait_list) < 0){
        return -3;
    }

    return 0;
}

/** @brief Allows a thread to wait for a condition variable.
 *
 *  @param cv The condition variable.
 *  @return Void.
 */
void cond_wait(cond_t *cv, mutex_t *mp)
{
    if (cv == NULL) {
        return;
    }

    if (mp)
        mutex_unlock(mp);

    waiter_t waiter = {gettid(), 0};

    spinlock_lock(&cv->wait_lock);
    linklist_add_tail(&cv->wait_list, (void*)&waiter);
    spinlock_unlock(&cv->wait_lock);

    deschedule_kern(&waiter.reject, false);

    if (mp)
        mutex_lock(mp);
}

/** @brief Wakes up a thread waiting on a condition variable.
 *
 *  @param cv The condition variable.
 *  @return Void.
 */
void cond_signal(cond_t *cv)
{
    if (cv == NULL) {
        return;
    }

    spinlock_lock(&cv->wait_lock);
    waiter_t *waiter;
    if (linklist_remove_head(&cv->wait_list, (void**)&waiter) == 0) {
        waiter->reject = 1;
        make_runnable_kern(waiter->tid, false);
    }
    spinlock_unlock(&cv->wait_lock);
}

/** @brief Wakes up all threads waiting on a condition variable.
 *
 *  @param cv The condition variable.
 *  @return Void.
 */
void cond_broadcast(cond_t *cv)
{
    if (cv == NULL) {
        return;
    }

    linklist_t list;

    spinlock_lock(&cv->wait_lock);
    linklist_move(&cv->wait_list, &list);
    spinlock_unlock(&cv->wait_lock);

    waiter_t *waiter;
    while (linklist_remove_head(&cv->wait_list, (void**)&waiter) == 0) {
        waiter->reject = 1;
        make_runnable_kern(waiter->tid, false);
    }
}