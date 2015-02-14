/** @file thr_internals.h
 *
 *  @brief This file may be used to define things
 *         internal to the thread library.
 */

#ifndef THR_INTERNALS_H
#define THR_INTERNALS_H

#include <mutex.h>
#include <hashtable.h>

#define HASH_TABLE_SIZE 32

typedef struct threadlib {
    int stack_size;
    void *stack_base;
    hashtable_t *threads;
    mutex_t *lock;
} threadlib_t;

typedef struct thread {
    int tid;
    void *esp3;
    void *stack_base;
} thread_t;

#endif /* THR_INTERNALS_H */
