/** @file spinlock.h
 *  @brief This file defines the interface for spinlocks.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _SPINLOCK_H
#define _SPINLOCK_H

#include <linklist.h>

typedef struct spinlock {
    int lock;
    int tid;
} spinlock_t;

/* Spinlock functions */
int spinlock_init(spinlock_t *sl);
void spinlock_lock(spinlock_t *sl);
void spinlock_unlock(spinlock_t *sl);

#endif /* _SPINLOCK_H */
