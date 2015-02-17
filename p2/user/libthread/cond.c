/** @file cond.c
 *  @brief This file implements the interface for condition variables.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <cond.h>
#include <syscall.h>
#include <stdlib.h>

/** @brief Initializes a condition variable.
 *
 *  @param cv The condition variable.
 *  @return 0 on success, number error code otherwise.
 */
int cond_init(cond_t *cv) {
    if (cv == NULL) {
        return -1;
    }

    if (linklist_init(cv->queue) < 0) {
        return -2;
    }

    if (mutex_init(cv->mutex) < 0) {
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
    if (cv == NULL) {
        return;
    }

    cv->valid = 0;

    mutex_destroy(cv->mutex);
}

/** @brief Allows a thread to wait for a condition variable and release the
 *  associated mutex.
 *
 *  @param cv The condition variable.
 *  @param mp The associated mutex.
 *  @return Void.
 */
void cond_wait(cond_t *cv, mutex_t *mp) {
    if (cv == NULL) {
        return;
    }

    if (!cv->valid) {
        return;
    }

    mutex_lock(cv->mutex);
    linklist_add_tail(cv->queue, (void*)gettid());
    mutex_unlock(cv->mutex);

    mutex_unlock(mp);

    int reject = 0;
    deschedule(&reject);

    mutex_lock(mp);
}

/** @brief Wakes up a thread waiting on a condition variable.
 *
 *  @param cv The condition variable.
 *  @return Void.
 */
void cond_signal(cond_t *cv) {
    if (cv == NULL) {
        return;
    }

    int tid;

    mutex_lock(cv->mutex);
    if(linklist_remove_head(cv->queue, (void**)&tid) < 0) {  
        mutex_unlock(cv->mutex);
        return;
    }
    mutex_unlock(cv->mutex);

    if (make_runnable(tid) == 0) {
        yield(tid);
    }
}

/** @brief Wakes up all threads waiting on a condition variable.
 *
 *  @param cv The condition variable.
 *  @return Void.
 */
void cond_broadcast(cond_t *cv) {
    if (cv == NULL) {
        return;
    }

    linklist_t list;

    mutex_lock(cv->mutex);
    if (linklist_move(cv->queue, &list) < 0) {
        return;
    }
    mutex_unlock(cv->mutex);

    int tid;
    while (linklist_remove_head(&list, (void**)&tid) == 0) {
        if (make_runnable(tid) == 0) {
            yield(tid);
        }
    }
}
