/** @file thread.c
 *  @brief This file implements the interface for the thread library.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <thread.h>
#include <thr_internals.h>
#include <autostack.h>
#include <syscall.h>
#include <stdlib.h>

extern stackinfo_t g_stackinfo;

static threadlib_t threadlib;

int thr_init(unsigned int size) {
    threadlib.stack_size = (size & PAGE_MASK);
    threadlib.stack_base = (void *)(g_stackinfo.stack_low + threadlib.stack_size);
    threadlib.threads = hashtable_init(HASH_TABLE_SIZE);
    if (threadlib.threads == NULL) {
        return -1;
    }
    if (mutex_init(threadlib.lock) < 0 ) {
        return -1;
    }
    
    return 0;
}

int thr_create(void *(*func)(void *), void *args) {
    thread_t *thread = (thread_t *)malloc(sizeof(thread_t));
    
    mutex_lock(threadlib.lock);
    
    thread->esp3 = threadlib.stack_base;
    if (new_pages(thread->esp3, PAGE_SIZE) < 0) {
        return -1;
    }
    threadlib.stack_base -= PAGE_SIZE;
    
    thread->stack_base = threadlib.stack_base;
    if (new_pages(thread->stack_base, PAGE_SIZE) < 0) {
        return -1;
    }
    threadlib.stack_base -= threadlib.stack_size;
    
    // Add thread_fork stuff here
    
    mutex_unlock(threadlib.lock);
    
    return 0;
}

int thr_join(int tid, void **statusp) {
    return 0;
}

void thr_exit(void *status) {

}

int thr_getid() {
    return gettid();
}

int thr_yield(int tid) {
    return 0;
}
