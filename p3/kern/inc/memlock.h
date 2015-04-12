/** @file memlock.h
 *  @brief This file defines the interface for memory locks.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _SVAR_H
#define _SVAR_H

#include <hashtable.h>
#include <mutex.h>
#include <cond.h>

typedef struct memlock {
    hashtable_t channel_ht;
    mutex_t channel_lock;
} memlock_t;

typedef enum {
	MEMLOCK_ACCESS,
	MEMLOCK_DESTROY
} memlock_type;

/* Memlock functions */
int memlock_init(memlock_t *memlock, int size);
int memlock_lock(memlock_t *memlock, void *ptr, memlock_type type);
void memlock_unlock(memlock_t *memlock, void *ptr);

#endif /* _SVAR_H */
