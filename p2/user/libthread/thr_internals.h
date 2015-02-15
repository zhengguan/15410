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
#define THREAD_N 4 //number of kernel threads

struct kernel_thread {
	int kernel_tid;
	int tid;
};

typedef struct threadlib {
	// linklist_t idle_threads;
    mutex_t lock;
		void *next_stack_base;
    hashtable_t *threads;
    unsigned stack_size;
    // struct kernel_thread kernel_threads[THREAD_N];
    int num_kernel_threads;
    int next_tid;
} threadlib_t;

typedef struct thread {
    ureg_t registers;
    void *stack_base;
    int tid;
    // int kernel_tid;

    int exited;
    int joiner_tid;
    void *status; //return value
} thread_t;

#endif /* THR_INTERNALS_H */
