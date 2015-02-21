/** @file thr_internals.h
 *
 *  @brief This file may be used to define things
 *         internal to the thread library.
 */

#ifndef THR_INTERNALS_H
#define THR_INTERNALS_H

#include <hashtable.h>
#include <mutex.h>
#include <ureg.h>

#define HASH_TABLE_SIZE 32
#define EXCEPTION_STACK_SIZE PAGE_SIZE

typedef struct threadlib {
    mutex_t mutex;
    unsigned int stack_size;
	void *next_stack_base;
    hashtable_t *threads;
} threadlib_t;

typedef struct thread {
    void *stack_base;
    void *esp3;
    int tid;
    int joiner_tid;
    int exited;
    void *status;
} thread_t;

#endif /* THR_INTERNALS_H */
