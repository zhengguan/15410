/** @file spinlock.c
 *  @brief This file implements spinlocks.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */


#include <spinlock.h>
#include <atom_xchg.h>
#include <stdlib.h>

/** @brief Initializes a spinlock.
 *
 *  @param sl The spinlock.
 *  @return 0 on success, negative error code otherwise.
 */
int spinlock_init(spinlock_t *sl) {
    if (sl == NULL) {
        return -1;
    }

    sl->lock = 0;
    sl->tid = -1;

    return 0;
}

/** @brief Locks a spinlock.  Blocks until lock is acquired.
 *
 *  @param sl The spinlock.
 *  @return Void.
 */
void spinlock_lock(spinlock_t *sl)
{
    if (sl == NULL) {
        return;
    }
    
    while (atom_xchg(&sl->lock, 1) != 0) {
        yield(sl->tid);
    }
    
    sl->tid = gettid();
}

/** @brief Unlocks a spinlock.
 *
 *  @param sl The spinlock.
 *  @return Void.
 */
void spinlock_unlock(spinlock_t *sl)
{    
    if (sl == NULL || !sl->lock || sl->tid != gettid()) {
        return;
    }
    
    sl->tid = -1;
    sl->lock = 0;
}
