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
 *  @param mp The mutex.
 *  @return 0 on success, negative error code otherwise.
 */
int mutex_init(mutex_t *mp) {
    if (mp == NULL) {
        return -1;
    }

    mp->lock = 0;
    mp->tid = -1;

    mp->valid = 1;

    return 0;
}

/** @brief "Deactivates" a mutex.
 *
 *  @param mp The mutex.
 *  @return Void.
 */
void mutex_destroy(mutex_t *mp) {
    if (mp == NULL) {
        return;
    }

    mp->valid = 0;
}

/** @brief Locks a mutex.
 *
 *  @param mp The mutex.
 *  @return Void.
 */
void mutex_lock(mutex_t *mp) {
    if (mp == NULL || mp->valid == 0) {
        return;
    }

    while (atom_xchg(&mp->lock, 1) != 0) {
        yield(mp->tid);
    }

    mp->tid = gettid();
}

/** @brief Unlocks a mutex.
 *
 *  @param mp The mutex.
 *  @return Void.
 */
void mutex_unlock(mutex_t *mp) {
    if (mp == NULL || mp->valid == 0 || mp->lock == 0 ||
        mp->tid != gettid()) {
        return;
    }

    mp->lock = 0;
}
