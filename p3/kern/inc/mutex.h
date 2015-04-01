/** @file mutex.h
 *  @brief This file defines the interface for mutexes.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _MUTEX_H
#define _MUTEX_H

#include <linklist.h>
#include <spinlock.h>

typedef struct mutex {
    int count;
    int tid;
    spinlock_t wait_lock;
    linklist_t wait_list;
} mutex_t;

/* Mutex functions */
int mutex_init(mutex_t *mp);
void mutex_lock(mutex_t *mp);
void mutex_unlock(mutex_t *mp);

#endif /* _MUTEX_H */
