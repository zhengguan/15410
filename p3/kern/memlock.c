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
#include <kern_common.h>

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

    if (mutex_init(&memlock->count_mutex) < 0){
        return -2;
    }

    if (mutex_init(&memlock->destroy_mutex) < 0){
        return -3;
    }

    if (cond_init(&memlock->destroy_cv) < 0){
        return -4;
    }

    if (hashtable_init(&memlock->count_ht, size) < 0){
        return -5;
    }

    return 0;
}

void memlock_lock(memlock_t *memlock, void *ptr, memlock_type type)
{
    if (memlock == NULL) {
        return;
    }

    ptr = (void *)ROUND_DOWN_PAGE(ptr);

    switch (type) {
        case MEMLOCK_ACCESS: {
            mutex_lock(&memlock->destroy_mutex);
            mutex_lock(&memlock->count_mutex);
            int count = 1;
            hashtable_add(&memlock->count_ht, (int)ptr, (void **)count);
            count++;
            hashtable_add(&memlock->count_ht, (int)ptr, (void *)count);
            mutex_unlock(&memlock->count_mutex);
            mutex_unlock(&memlock->destroy_mutex);
            break;
        }

        case MEMLOCK_DESTROY: {
            mutex_lock(&memlock->destroy_mutex);
            mutex_lock(&memlock->count_mutex);
            if (hashtable_get(&memlock->count_ht, (int)ptr, NULL) < 0) {
                cond_wait(&memlock->destroy_cv, &memlock->count_mutex);
            }
            mutex_unlock(&memlock->count_mutex);
            break;
        }
        
        default: {
            break;
        }
    }
}

void memlock_unlock(memlock_t *memlock, void *ptr)
{
    if (memlock == NULL) {
        return;
    }

    ptr = (void *)ROUND_DOWN_PAGE(ptr);

    mutex_lock(&memlock->count_mutex);
    int count;
    if (hashtable_get(&memlock->count_ht, (int)ptr, (void **)&count) == 0) {
        count--;
        if (count == 0) {
            cond_signal(&memlock->destroy_cv);
        } else {
            hashtable_add(&memlock->count_ht, (int)ptr, (void *)count);
        }
    } else {
        mutex_unlock(&memlock->destroy_mutex);
    }
    mutex_unlock(&memlock->count_mutex);
}
