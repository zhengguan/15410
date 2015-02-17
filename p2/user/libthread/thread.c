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

#include <simics.h>

static threadlib_t threadlib;

extern mutex_t malloc_mutex; // FIXME: put this here?

typedef struct stack_top {
    void *return_address; //NULL. This is where the stack pointer points on the function call
    void *function;
    void *argument; //this is at thread->stack_base
    void *esp3;
} stack_top_t;


static void exception_handler(void *esp3, ureg_t *ureg)
{
    task_vanish(-ureg->cause);
    //FIXME: remove this if necessary
    swexn(esp3, exception_handler, esp3, ureg);
}

int thr_getid()
{
    return gettid();
}

static int record_main_thread()
{
    thread_t *thread = (thread_t *)malloc(sizeof(thread_t));

    thread->tid = thr_getid();

    thread->stack_base = g_stackinfo.stack_high;

    thread->joiner_tid = -1;
    thread->exited = 0;

    thread->esp3 = ROOT_HANDLER_STACK;

    swexn(thread->esp3, exception_handler, thread->esp3, NULL);

    hashtable_add(threadlib.threads, thread->tid, thread);

    if (new_pages(g_stackinfo.stack_low-threadlib.stack_size, threadlib.stack_size) < 0) {
        lprintf("new pages failed: %p, %u", g_stackinfo.stack_low-threadlib.stack_size, threadlib.stack_size);
        return -1;
    }
    return 0;
}

static void thread_wrapper(void *(*func)(void*), void *arg, void *esp3)
{
    swexn(esp3, exception_handler, esp3, NULL);
    void *ret = func(arg);

    thr_exit(ret);
}

int thr_init(unsigned size)
{
    //round up to next page
    threadlib.stack_size = ((size + PAGE_SIZE-1) & PAGE_MASK);
    threadlib.next_stack_base = (void *)(((unsigned)g_stackinfo.stack_low - threadlib.stack_size) & PAGE_MASK);
    threadlib.threads = hashtable_init(HASH_TABLE_SIZE);

    if (threadlib.threads == NULL) {
        return -1;
    }

    if (record_main_thread() < 0)
        return -4;

    if (mutex_init(&threadlib.lock) < 0) {
        return -2;
    }

    //TODO: where do i go???
    if (mutex_init(&malloc_mutex) < 0) {
        return -3;
    }

    return 0;
}


int thr_create(void *(*func)(void *), void *args)
{
    thread_t *thread = (thread_t *)malloc(sizeof(thread_t));
    if (thread == NULL)
        return -1;

    mutex_lock(&threadlib.lock);

    thread->esp3 = threadlib.next_stack_base;
    threadlib.next_stack_base -= EXCEPTION_STACK_SIZE;
    thread->stack_base = threadlib.next_stack_base;
    threadlib.next_stack_base -= threadlib.stack_size;

    unsigned eip = (unsigned)thread_wrapper;
    unsigned esp = (unsigned)&((stack_top_t *)thread->stack_base)->return_address;

    int total_stack_size = EXCEPTION_STACK_SIZE + threadlib.stack_size;

    //allocate the new stack
    if (new_pages(thread->esp3-total_stack_size, total_stack_size) < 0) {
        lprintf("new pages failed: %p, %u", thread->esp3-total_stack_size, total_stack_size);
        return -2;
    }

    //initialize stack
    ((stack_top_t *)thread->stack_base)->esp3 = thread->esp3;
    ((stack_top_t *)thread->stack_base)->argument = args;
    ((stack_top_t *)thread->stack_base)->function = func;
    ((stack_top_t *)thread->stack_base)->return_address = NULL;

    thread->joiner_tid = -1;
    thread->exited = 0;

    int tid = new_kernel_thread(eip, esp);
    thread->tid = tid;
    hashtable_add(threadlib.threads, thread->tid, thread);
    mutex_unlock(&threadlib.lock);

    return tid;
}


int thr_join(int tid, void **statusp)
{
    mutex_lock(&threadlib.lock);
    thread_t *thread;
    hashtable_get(threadlib.threads, tid, (void **)&thread);
    if (!thread) {
        mutex_unlock(&threadlib.lock);
        return -1;
    }

    if (thread->joiner_tid > 0) {
        mutex_unlock(&threadlib.lock);
        return -2;
    }

    thread->joiner_tid = thr_getid();

    mutex_unlock(&threadlib.lock);

    //wait for thread to exit
    while (!thread->exited) {
        thr_yield(tid);
    }

    mutex_lock(&threadlib.lock);

    hashtable_remove(threadlib.threads, tid);

    mutex_unlock(&threadlib.lock);

    if (statusp)
        *statusp = thread->status;

    free(thread);

    return 0;
}

void thr_exit(void *status)
{
    mutex_lock(&threadlib.lock);
    thread_t *thread;
    hashtable_get(threadlib.threads, thr_getid(), (void **)&thread);
    if (thread != NULL) {
        //otherwise error
        thread->status = status;
        thread->exited = 1;
    }

    mutex_unlock(&threadlib.lock);
    vanish();
}


int thr_yield(int tid)
{
    return yield(tid);
}
