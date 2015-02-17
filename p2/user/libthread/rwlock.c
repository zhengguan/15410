/** @file rwlock.c
 *  @brief This file implements the interface for reader-writer locks.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <rwlock.h>
#include <stdlib.h>

/**
 * @brief Initializes the rwlock.
 * @param rwlock The rwlock.
 * @return 0 on success, negative error code otherwise.
 */
int rwlock_init(rwlock_t *rwlock)
{
	if (rwlock == NULL) {
		return -1;
    }
    
	rwlock->reader_count = 0;

	if (mutex_init(&rwlock->writer_mutex) < 0) {
		return -2;
	}
	
	if (mutex_init(&rwlock->reader_count_mutex) < 0) {
		return -3;
	}
	
	if (cond_init(&rwlock->cond) < 0) {
		return -4;
	}
	
	rwlock->valid = 1;
		
	return 0;
}

/**
 * @brief Locks the rwlock.
 *
 * @param rwlock The rwlock.
 * @param type The type of lock to acquire.
 * @return Void.
 */
void rwlock_lock(rwlock_t *rwlock, int type)
{
	switch (type) {
		case RWLOCK_READ: {
			mutex_lock(&rwlock->writer_mutex);
			mutex_lock(&rwlock->reader_count_mutex);
			rwlock->reader_count++;
			mutex_unlock(&rwlock->reader_count_mutex);
			mutex_unlock(&rwlock->writer_mutex);
			break;
		}

		case RWLOCK_WRITE: {
			mutex_lock(&rwlock->writer_mutex);
			mutex_lock(&rwlock->reader_count_mutex);
			cond_wait(&rwlock->cond, &rwlock->reader_count_mutex);
			mutex_unlock(&rwlock->reader_count_mutex);
			break;
		}
		
		default: {
		    break;
	    }
	}
}

/**
 * @brief Unlocks the rwlock
 *
 * @param rwlock The rwlock.
 * @return Void.
 */
void rwlock_unlock(rwlock_t *rwlock)
{
	mutex_lock(&rwlock->reader_count_mutex);
	if (rwlock->reader_count > 0) {
		rwlock->reader_count--;
		if (rwlock->reader_count == 0)
			cond_signal(&rwlock->cond);
	} else {
		mutex_unlock(&rwlock->writer_mutex);
	}
	mutex_unlock(&rwlock->reader_count_mutex);
}

/**
 * @brief "Deactivates" a rwlock.
 *
 * @param rwlock The rwlock.
 * @return Void.
 */
void rwlock_destroy(rwlock_t *rwlock)
{
	rwlock->valid = 0;
	
	mutex_destroy(&rwlock->writer_mutex);
	mutex_destroy(&rwlock->reader_count_mutex);
	cond_destroy(&rwlock->cond);
}

/**
 * @brief Downgrades a rwlock.
 * @details Can only be called by the thread that holds the writer lock.
 * Turns the writer in to a reader.
 *
 * @param rwlock The rwlock.
 * @return Void.
 */
void rwlock_downgrade(rwlock_t *rwlock) {
	mutex_lock(&rwlock->reader_count_mutex);
	rwlock->reader_count++;
	mutex_unlock(&rwlock->reader_count_mutex);
	mutex_unlock(&rwlock->writer_mutex);
}
