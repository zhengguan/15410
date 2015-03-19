/** @file mutex.h
 *  @brief This file defines the interface for mutexes.
 */

#ifndef _MUTEX_H
#define _MUTEX_H

#include <linklist.h>

typedef struct mutex {
    int valid;
    int lock;
    int tid;
    // TODO possible move out to spinlock.c implementation
    int wait_lock;
    linklist_t wait_list;
} mutex_t;

/* Mutex functions */
int mutex_init(mutex_t *mp);
void mutex_destroy(mutex_t *mp);
void mutex_lock(mutex_t *mp);
void mutex_unlock(mutex_t *mp);

#endif /* _MUTEX_H */
