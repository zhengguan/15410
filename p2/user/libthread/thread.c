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
#include <rand.h>
#include <new_kernel_thread.h>

static threadlib_t threadlib;

typedef struct stacktop {
    void *return_address;
    void *function;
    void *argument;
    void *esp3;
} stacktop_t;

//FIXME: what to do here?
static void exception_handler(void *esp3, ureg_t *ureg)
{
    task_vanish(-ureg->cause);
    
    swexn(esp3, exception_handler, esp3, ureg);
}

static void thread_wrapper(void *(*func)(void*), void *arg, void *esp3)
{
    swexn(esp3, exception_handler, esp3, NULL);
    
    void *ret = func(arg);

    thr_exit(ret);
}

static int add_main_thread()
{
    thread_t *thread = (thread_t *)malloc(sizeof(thread_t));
    if (thread == NULL) {
        return -1;
    }

    if (new_pages(threadlib.next_stack_base-threadlib.stack_size, threadlib.stack_size) < 0) {
        free(thread);
        return -2;
    }
    thread->stack_base = threadlib.next_stack_base;
    threadlib.next_stack_base -= threadlib.stack_size;
    thread->esp3 = ROOT_HANDLER_STACK;
    
    thread->tid = thr_getid();
    thread->joiner_tid = -1;
    thread->exited = 0;

    hashtable_add(threadlib.threads, thread->tid, thread);
    
    swexn(thread->esp3, exception_handler, thread->esp3, NULL);
    
    return 0;
}

int thr_init(unsigned int size)
{
    threadlib.stack_size = ((size + PAGE_SIZE-1) & PAGE_MASK);
    threadlib.next_stack_base = g_stackinfo.stack_low;
    threadlib.threads = hashtable_init(HASH_TABLE_SIZE);

    if (threadlib.threads == NULL) {
        return -1;
    }
    
    if (mutex_init(&threadlib.mutex) < 0) {
        return -2;
    }

    if (add_main_thread() < 0) {
        return -3;
    }

    return 0;
}

int thr_create(void *(*func)(void *), void *args)
{
    thread_t *thread = (thread_t *)malloc(sizeof(thread_t));
    if (thread == NULL) {
        return -1;
    }

    mutex_lock(&threadlib.mutex);

    int total_stack_size = EXCEPTION_STACK_SIZE + threadlib.stack_size;
    if (new_pages(threadlib.next_stack_base-total_stack_size, total_stack_size) < 0) {
        free(thread);
        return -2;
    }
    thread->stack_base = threadlib.next_stack_base;
    threadlib.next_stack_base -= threadlib.stack_size;
    thread->esp3 = threadlib.next_stack_base;
    threadlib.next_stack_base -= EXCEPTION_STACK_SIZE;

    ((stacktop_t *)thread->stack_base)->esp3 = thread->esp3;
    ((stacktop_t *)thread->stack_base)->argument = args;
    ((stacktop_t *)thread->stack_base)->function = func;
    ((stacktop_t *)thread->stack_base)->return_address = NULL;

    unsigned int eip = (unsigned int)thread_wrapper;
    unsigned int esp = (unsigned int)&((stacktop_t *)thread->stack_base)->return_address;
    int tid = new_kernel_thread(eip, esp);
    thread->tid = tid;
    
    thread->joiner_tid = -1;
    thread->exited = 0;
    
    hashtable_add(threadlib.threads, thread->tid, thread);
    
    mutex_unlock(&threadlib.mutex);

    return tid;
}


int thr_join(int tid, void **statusp)
{
    mutex_lock(&threadlib.mutex);
    
    thread_t *thread;
    if (hashtable_get(threadlib.threads, tid, (void **)&thread) < 0) {
        mutex_unlock(&threadlib.mutex);
        return -1;
    }

    if (thread->joiner_tid > 0) {
        mutex_unlock(&threadlib.mutex);
        return -2;
    }

    thread->joiner_tid = thr_getid();

    mutex_unlock(&threadlib.mutex);

    while (!thread->exited) {
        thr_yield(tid);
    }

    mutex_lock(&threadlib.mutex);

    hashtable_remove(threadlib.threads, tid);

    mutex_unlock(&threadlib.mutex);

    if (statusp != NULL) {
        *statusp = thread->status;
    }
    
    free(thread);

    return 0;
}

void thr_exit(void *status)
{
    mutex_lock(&threadlib.mutex);
    
    thread_t *thread;
    if (hashtable_get(threadlib.threads, thr_getid(), (void **)&thread) == 0) {
        thread->status = status;
        thread->exited = 1;
    }

    mutex_unlock(&threadlib.mutex);
    
    vanish();
}

int thr_getid()
{
    return gettid();
}

int thr_yield(int tid)
{
    return yield(tid);
}

