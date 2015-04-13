/** @file memlock.c
 *  @brief This file implements the interface for memory locks.
 *
 *  Memlocks are similar to reader writer locks but for virtual memory pages.
 *  When locking, callers must specify a virtual address they wish to lock and
 *  the type of access they need.  Locking or unlocking a page in memory does
 *  not affect locking or unlocking for any other pages
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
#include <kern_common.h>
#include <simics.h>

typedef struct {
    rwlock_t lock;
    mutex_t waiters_lock;
    int waiters;
} memlock_channel_t;

/** @brief Initializes a memlock.
 *
 *  @param memlock The memlock.
 *  @param size The size of the wait hashtable.
 *  @return 0 on success, negative error code otherwise.
 */
int memlock_init(memlock_t *memlock, int size)
{
    if (memlock == NULL) {
        return -1;
    }

    if (mutex_init(&memlock->channel_lock) < 0){
        return -2;
    }

    if (hashtable_init(&memlock->channel_ht, size) < 0){
        return -3;
    }

    return 0;
}

/** @brief Locks a memlock.
 *
 *  @param memlock The memlock.
 *  @param ptr The memory address to lock.
 *  @param type The type of lock to acquire.
 *  @return 0 on success, negative error code otherwise.
 */
int memlock_lock(memlock_t *memlock, void *ptr, memlock_type type)
{
    if (memlock == NULL) {
        return -1;
    }

    ptr = (void *)ROUND_DOWN_PAGE(ptr);

    mutex_lock(&memlock->channel_lock);
    memlock_channel_t *channel;
    if (hashtable_get(&memlock->channel_ht, (int)ptr, (void **)&channel) < 0)
    {
        channel = malloc(sizeof(memlock_channel_t));
        if (channel == NULL) {
            mutex_unlock(&memlock->channel_lock);
            return -2;
        }
        rwlock_init(&channel->lock);
        mutex_init(&channel->waiters_lock);
        hashtable_add(&memlock->channel_ht, (int)ptr, (void *)channel);
    }
    //indicate that we are waiting for the lock so that it is not freed.
    mutex_lock(&channel->waiters_lock);
    channel->waiters++;
    mutex_unlock(&channel->waiters_lock);
    mutex_unlock(&memlock->channel_lock);

    switch (type) {
        case MEMLOCK_ACCESS: {
            rwlock_lock(&channel->lock, RWLOCK_READ);
            break;
        }

        case MEMLOCK_DESTROY: {
            rwlock_lock(&channel->lock, RWLOCK_WRITE);
            break;
        }
    }
    return 0;
}

/** @brief Unlocks a memlock.
 *
 *  @param memlock The memlock.
 *  @param ptr The memory address to unlock.
 *  @return Void.
 */
void memlock_unlock(memlock_t *memlock, void *ptr)
{

    if (memlock == NULL) {
        return;
    }

    ptr = (void *)ROUND_DOWN_PAGE(ptr);
    mutex_lock(&memlock->channel_lock);
    memlock_channel_t *channel;
    if (hashtable_get(&memlock->channel_ht, (int)ptr, (void **)&channel) < 0) {
        mutex_unlock(&memlock->channel_lock);
        return;
    }

    rwlock_unlock(&channel->lock);

    mutex_lock(&channel->waiters_lock);
    if (--channel->waiters == 0) {
        hashtable_remove(&memlock->channel_ht, (int)ptr, NULL);
        mutex_unlock(&channel->waiters_lock);
        free(channel);
    }
    mutex_unlock(&memlock->channel_lock);

}
