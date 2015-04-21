/** @file rwlock_type.h
 *  @brief This file defines the type for reader/writer locks.
 */

#ifndef _RWLOCK_TYPE_H
#define _RWLOCK_TYPE_H

#include <mutex.h>
#include <cond.h>

typedef struct rwlock {
	int valid;
	int reader_count;
	mutex_t reader_count_mutex;
	mutex_t writer_mutex;
	cond_t cond;
} rwlock_t;

#endif /* _RWLOCK_TYPE_H */
