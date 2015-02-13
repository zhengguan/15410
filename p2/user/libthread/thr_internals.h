/** @file thr_internals.h
 *
 *  @brief This file may be used to define things
 *         internal to the thread library.
 */



#ifndef THR_INTERNALS_H
#define THR_INTERNALS_H

#define PAGE_MASK ((unsigned int)(~(PAGE_SIZE - 1)))

#define HASH_TABLE_SIZE = 32;

typedef struct threadlib {
    int stack_size;
    void *stack_base;
    hashtable_t threads;
} threadlib_t;

typedef struct thread {
    int tid;
    void *esp3;
    void *stack_low;
    void *stack_high;
} thread_t


#endif /* THR_INTERNALS_H */
