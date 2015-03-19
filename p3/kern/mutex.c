/** @file mutex.c
 *  @brief This file implements mutexes.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <mutex.h>
#include <atom_xchg.h>
#include <stdlib.h>
#include <syscall.h>
#include <linklist.h>

typedef struct waiter {
    int tid;
    int reject;
} waiter_t;

/** @brief Initializes a mutex.
 *
 *  @param mp The mutex.
 *  @return 0 on success, negative error code otherwise.
 */
int mutex_init(mutex_t *mp) {
    if (mp == NULL) {
        return -1;
    }

    mp->lock = 0;
    mp->wait_lock = 0;
    linklist_init(&mp->wait_list);

    return 0;
}

/** @brief "Deactivates" a mutex.
 *
 *  @param mp The mutex.
 *  @return Void.
 */
void mutex_destroy(mutex_t *mp) {
    if (mp == NULL || !mp->valid) {
        return;
    }

    mp->valid = 0;
}

/** @brief Locks a mutex.  Blocks until mutex is acquired.
 *
 *  @param mp The mutex.
 *  @return Void.
 */
void mutex_lock(mutex_t *mp) {
    if (mp == NULL || !mp->valid) {
        return;
    }
    
    int tid = gettid();
    waiter_t waiter = {tid, 0};
    while (atom_xchg(&mp->wait_lock, 1) != 0) {
        // FIXME restore this, should this yield to the thread with the mutex?
        // yield(-1);
    }
    linklist_add_tail(&mp->wait_list, (void*)&waiter);
    mp->wait_lock = 0;

    // FIXME
    // deschedule(&waiter.reject);
   
    mp->tid = tid;
}

/** @brief Unlocks a mutex.
 *
 *  @param mp The mutex.
 *  @return Void.
 */
void mutex_unlock(mutex_t *mp) {
    if (mp == NULL || !mp->lock || mp->tid != gettid()) {
        return;
    }

    waiter_t *waiter;
    while (atom_xchg(&mp->wait_lock, 1) != 0) {
        // FIXME restore this, should this yield to the thread with the mutex?
        // yield(-1);
    }
    if(linklist_remove_head(&mp->wait_list, (void**)&waiter) < 0) {
        mp->wait_lock = 0;
        return;
    }
    // int tid = waiter->tid;
    waiter->reject = 1;
    mp->wait_lock = 0;

    // FIXME restore this (and uncomment line above)
    // make_runnable(tid);
    
    // TODO Should we yield to tid here?
}

