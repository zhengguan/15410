/** @file mutex.c
 *  @brief This file implements the interface for mutexes.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <mutex.h>
#include <atom_xchg.h>
#include <syscall.h>
#include <stdlib.h>

/** @brief Initializes a mutex.
 *
 *  @param list Mutex.
 *  @return 0 on success, number less than 0 on error.
 */
int mutex_init(mutex_t *mp) {
    if (mp == NULL) {
        return -1;
    }

    mp->lock = 0;
    mp->tid = -1;
    mp->count = 0;
    mp->count_lock = 0;

    mp->active_flag = 1;

    return 0;
}

/** @brief "Deactivates" a mutex.
 *
 *  @param list Mutex.
 *  @return Void.
 */
void mutex_destroy(mutex_t *mp) {
    if (mp == NULL) {
        return;
    }

    mp->active_flag = 0;

    while (mp->count > 0) {
        yield(mp->tid);
    }
}

/** @brief Locks a mutex.
 *
 *  @param list Mutex.
 *  @return Void.
 */
void mutex_lock(mutex_t *mp) {
    if (mp == NULL || mp->active_flag == 0) {
        return;
    }

    while (atom_xchg(&mp->count_lock, 1) != 0) {
        yield(-1);
    }
    mp->count++;
    mp->count_lock = 0;

    while (atom_xchg(&mp->lock, 1) != 0) {
        yield(mp->tid);
    }

    mp->tid = gettid();
}

/** @brief Unlocks a mutex.
 *
 *  @param list Mutex.
 *  @return Void.
 */
void mutex_unlock(mutex_t *mp) {
    if (mp == NULL || mp->active_flag == 0 || mp->lock == 0 ||
        mp->tid != gettid()) {
        return;
    }

    while (atom_xchg(&mp->count_lock, 1) != 0) {
        yield(-1);
    }
    mp->count--;
    mp->count_lock = 0;

    mp->lock = 0;
}
