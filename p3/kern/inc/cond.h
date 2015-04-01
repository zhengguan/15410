/** @file mutex.h
 *  @brief This file defines the interface for condition variables.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _COND_H
#define _COND_H

#include <linklist.h>
#include <spinlock.h>

typedef struct cond {
    spinlock_t wait_lock;
    linklist_t wait_list;
} cond_t;

/* Condition variable functions */
int cond_init( cond_t *cv );
void cond_wait( cond_t *cv);
void cond_signal( cond_t *cv );
void cond_broadcast( cond_t *cv );

#endif /* _COND_H */
