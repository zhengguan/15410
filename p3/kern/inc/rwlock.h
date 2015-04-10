/** @file rwlock_type.h
 *  @brief This file defines the type for reader/writer locks.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _RWLOCK_TYPE_H
#define _RWLOCK_TYPE_H

#include <mutex.h>
#include <cond.h>

typedef struct rwlock {
    int reader_count;
    mutex_t reader_count_mutex;
    mutex_t writer_mutex;
    cond_t cond;
} rwlock_t;

typedef enum {
	RWLOCK_READ,
	RWLOCK_WRITE
} rwlock_type;

/* readers/writers lock functions */
int rwlock_init(rwlock_t *rwlock);
void rwlock_lock(rwlock_t *rwlock, rwlock_type type);
void rwlock_unlock(rwlock_t *rwlock);
void rwlock_destroy(rwlock_t *rwlock);
void rwlock_downgrade(rwlock_t *rwlock);

#endif /* _RWLOCK_TYPE_H */