/** @file thr_internals.h
 *
 *  @brief This file may be used to define things
 *         internal to the thread library.
 */

#ifndef THR_INTERNALS_H
#define THR_INTERNALS_H

#include <mutex.h>
#include <hashtable.h>
#include <ureg.h>

#define HASH_TABLE_SIZE 32

typedef struct threadlib {
    mutex_t lock;
	void *next_stack_base;
    hashtable_t *threads;
    unsigned stack_size;
} threadlib_t;

typedef struct thread {
    void *stack_base;
    int tid;

    int exited; //boolean
    int joiner_tid;
    void *status; //return value
} thread_t;

#endif /* THR_INTERNALS_H */
