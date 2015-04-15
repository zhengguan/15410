/** @file memlock.h
 *  @brief This file defines the interface for memory locks.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _MEMLOCK_H
#define _MEMLOCK_H

#include <hashtable.h>
#include <mutex.h>
#include <cond.h>

typedef struct memlock {
    hashtable_t channel_ht;
    mutex_t channel_lock;
} memlock_t;

typedef enum {
	MEMLOCK_ACCESS,
	MEMLOCK_MODIFY
} memlock_type;

/* Memlock functions */
int memlock_init(memlock_t *memlock, int size);
int memlock_lock(memlock_t *memlock, void *ptr, memlock_type type);
void memlock_unlock(memlock_t *memlock, void *ptr);
void memlock_destroy(memlock_t *memlock);

#endif /* _MEMLOCK_H */
