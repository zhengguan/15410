/** @file cond.c
 *  @brief This file implements the interface for condition variables.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <cond.h>
#include <thread.h>
#include <syscall.h>
#include <stdlib.h>

typedef struct waiter {
    int tid;
    int reject;
} waiter_t;

/** @brief Initializes a condition variable.
 *
 *  @param cv The condition variable.
 *  @return 0 on success, number error code otherwise.
 */
int cond_init(cond_t *cv) {
    if (cv == NULL) {
        return -1;
    }

    if (linklist_init(&cv->queue) < 0) {
        return -2;
    }

    if (mutex_init(&cv->mutex) < 0) {
        return -3;
    }

    cv->valid = 1;

    return 0;
}

/** @brief "Deactivates" a condition variable.
 *
 *  @param cv The condition variable.
 *  @return Void.
 */
void cond_destroy(cond_t *cv) {
    if (cv == NULL || !cv->valid) {
        return;
    }

    cv->valid = 0;

    mutex_destroy(&cv->mutex);
}

/** @brief Allows a thread to wait for a condition variable and release the
 *  associated mutex.
 *
 *  @param cv The condition variable.
 *  @param mp The associated mutex.
 *  @return Void.
 */
void cond_wait(cond_t *cv, mutex_t *mp) {
    if (cv == NULL || !cv->valid) {
        return;
    }

    waiter_t waiter = {thr_getid(), 0};

    mutex_lock(&cv->mutex);
    linklist_add_tail(&cv->queue, (void*)&waiter);
    mutex_unlock(&cv->mutex);

    mutex_unlock(mp);

    deschedule(&waiter.reject);

    mutex_lock(mp);
}

/** @brief Wakes up a thread waiting on a condition variable.
 *
 *  @param cv The condition variable.
 *  @return Void.
 */
void cond_signal(cond_t *cv) {
    if (cv == NULL || !cv->valid) {
        return;
    }

    int tid;
    waiter_t *waiter;

    mutex_lock(&cv->mutex);
    if(linklist_remove_head(&cv->queue, (void**)&waiter) < 0) {
        mutex_unlock(&cv->mutex);
        return;
    }
    tid = waiter->tid;
    waiter->reject = 1;
    mutex_unlock(&cv->mutex);

    make_runnable(tid);
    yield(tid);
}

/** @brief Wakes up all threads waiting on a condition variable.
 *
 *  @param cv The condition variable.
 *  @return Void.
 */
void cond_broadcast(cond_t *cv) {
    if (cv == NULL || !cv->valid) {
        return;
    }

    linklist_t list;

    mutex_lock(&cv->mutex);
    if (linklist_move(&cv->queue, &list) < 0) {
        return;
    }
    mutex_unlock(&cv->mutex);

    int tid;
    waiter_t *w;
    while (linklist_remove_head(&list, (void**)&w) == 0) {
        tid = w->tid;
        w->reject = 1;
        make_runnable(tid);
    }
    yield(-1);
}
