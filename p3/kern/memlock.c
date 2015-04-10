/** @file memlock.c
 *  @brief This file implements the interface for memory locks.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <memlock.h>
#include <stdlib.h>
#include <syscall.h>
#include <waiter.h>
#include <scheduler.h>

/** @brief Initializes a memlock.
 *
 *  @param memlock The memlock.
 *  @param size The size of the wait hashtable.
 *  @return 0 on success, number error code otherwise.
 */
int memlock_init(memlock_t *memlock, int size)
{
    if (memlock == NULL) {
        return -1;
    }

    if (spinlock_init(&memlock->wait_lock) < 0) {
        return -2;
    }

    if (hashtable_init(&memlock->wait_ht, size) < 0){
        return -3;
    }

    return 0;
}

/** @brief Allows a thread to wait for a memlock.
 *
 *  @param memlock The memlock
 *  @oaram mp The mutex.
 *  @return Void.
 */
void memlock_wait(memlock_t *memlock, void *ptr, mutex_t *mp)
{
    if (memlock == NULL) {
        return;
    }

    if (mp != NULL) {
        mutex_unlock(mp);
    }

    waiter_t waiter = {gettcb(), 0};

    spinlock_lock(&memlock->wait_lock);
    hashtable_add(&memlock->wait_ht, (int)ptr, (void*)&waiter);
    spinlock_unlock(&memlock->wait_lock);

    deschedule_kern(&waiter.reject, false);

    if (mp != NULL) {
        mutex_lock(mp);
    }
}

/** @brief Wakes up all threads waiting on a memlock.
 *
 *  @param memlock The memlock.
 *  @return Void.
 */
void memlock_broadcast(memlock_t *memlock, void *ptr)
{
    if (memlock == NULL) {
        return;
    }

    spinlock_lock(&memlock->wait_lock);
    waiter_t *waiter;
    while (hashtable_remove(&memlock->wait_ht, (int)ptr, (void**)&waiter) == 0) {
        waiter->reject = 1;
        make_runnable_kern(waiter->tcb, false);
    }
    spinlock_unlock(&memlock->wait_lock);
}
