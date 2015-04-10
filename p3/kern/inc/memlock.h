/** @file memlock.h
 *  @brief This file defines the interface for memory locks.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _SVAR_H
#define _SVAR_H

#include <spinlock.h>
#include <hashtable.h>
#include <mutex.h>

typedef struct memlock {
    spinlock_t wait_lock;
    hashtable_t wait_ht;
} memlock_t;

/* Memlock functions */
int memlock_init(memlock_t *memlock, int size);
void memlock_wait(memlock_t *memlock, void *ptr, mutex_t *mp);
void memlock_broadcast(memlock_t *memlock, void *ptr);

#endif /* _SVAR_H */
