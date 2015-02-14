/** @file rwlock_type.h
 *  @brief This file defines the type for reader/writer locks.
 */

#ifndef _RWLOCK_TYPE_H
#define _RWLOCK_TYPE_H

#include <mutex.h>

typedef struct rwlock {
	int valid;
	int num_readers;
	mutex_t num_readers_mutex;
	mutex_t writer_mutex;
	cond_t cond;
} rwlock_t;

#endif /* _RWLOCK_TYPE_H */
