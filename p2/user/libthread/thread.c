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
extern mutex_t malloc_mutex; // FIXME: here?

// //must be locked
// static kernel_thread *find_kernel_thread(int kernel_tid)
// {
//     int i;
//     for (i = 0; i < threadlib.num_kernel_threads; i++)
//         if (threadlib.kernel_threads[i].kernel_tid = kernel_tid)
//             return threadlib.kernel_threads + i;
//     return -1;
// }


static void record_main_thread()
{
    thread_t *thread = (thread_t *)malloc(sizeof(thread_t));

    thread->tid = gettid();//threadlib.next_tid++;

    thread->stack_base = g_stackinfo.stack_high;

    // thread->kernel_tid = gettid();

    thread->joiner_tid = -1;
    thread->exited = 0;

    //update the mapping b/w tids
    // find_kernel_thread(thread->kernel_tid)->tid = thread->tid;

    hashtable_add(threadlib.threads, thread->tid, thread);
}

int thr_init(unsigned size)
{
    lprintf("initializing threadlib with size %u", size);
    //round up to next page
    threadlib.stack_size = ((size + PAGE_SIZE-1) & PAGE_MASK);
    threadlib.next_stack_base = (void *)(((unsigned)g_stackinfo.stack_high - threadlib.stack_size) & PAGE_MASK);
    threadlib.threads = hashtable_init(HASH_TABLE_SIZE);
    // threadlib.next_tid = 1;

    // threadlib.kernel_threads = {-1};

    if (threadlib.threads == NULL) {
        return -1;
    }

    record_main_thread();

    sgenrand(get_ticks());


    if (mutex_init(&threadlib.lock) < 0) {
        return -2;
    }

    if (mutex_init(&malloc_mutex) < 0) {
        return -3;
    }

    // if (linklist_init(&threadlib.idle_threads) < 0) {
    //     return -4;
    // }

    return 0;
}

//should be locked
static inline int choose_kernel_thread()
{
    return genrand() % threadlib.num_kernel_threads;
}

typedef struct stack_top {
    void *argument; //this is at thread->stack_base
    void *return_address; //NULL
    void *saved_ebp; //NULL. This is where the stack pointer points on the function call
} stack_top_t;

int thr_create(void *(*func)(void *), void *args)
{
    lprintf("creating thread with func %p and arg %p", func, args);

    thread_t *thread = (thread_t *)malloc(sizeof(thread_t));
    if (thread == NULL)
        return -1;

    mutex_lock(&threadlib.lock);
    lprintf("%d", 0);

    // thread->tid = threadlib.next_tid++;

    thread->stack_base = threadlib.next_stack_base;
    threadlib.next_stack_base -= threadlib.stack_size;
    lprintf("new stack region: [%p, %p)", thread->stack_base, threadlib.next_stack_base);

    //FIXME: does this need to do something to cs register?
    thread->registers.eip = (unsigned)func;

    lprintf("%d", 2);


    //setup the new stack
    if (new_pages(thread->stack_base-threadlib.stack_size, threadlib.stack_size) < 0) {
        lprintf("new pages failed: %p, %u", thread->stack_base-threadlib.stack_size, threadlib.stack_size);
    }

    ((stack_top_t *)thread->stack_base)->argument = args;
    ((stack_top_t *)thread->stack_base)->return_address = NULL;
    ((stack_top_t *)thread->stack_base)->saved_ebp = NULL;
    thread->registers.esp = (unsigned)&((stack_top_t *)thread->stack_base)->saved_ebp;

    thread->joiner_tid = -1;
    thread->exited = 0;

    lprintf("%d", 3);


    // linklist_add_tail(&threadlib.idle_threads, thread);

    // if (num_kernel_threads < THREAD_N) {
    //     //choose new thread to run


    // lprintf("%p, %d, %d",thread->stack_base, threadlib.stack_size, PAGE_SIZE);
    // if (new_pages(thread->stack_base, threadlib.stack_size) < 0) {
    //     lprintf("wttf");
    //     return -2;
    // }


    lprintf("about to split");
    int tid = new_kernel_thread(thread->registers.eip, thread->registers.esp);
    lprintf("done split");
    thread->tid = tid;
    hashtable_add(threadlib.threads, thread->tid, thread);
    // }
    mutex_unlock(&threadlib.lock);

    return tid;
}

int thr_getid()
{
    return gettid();
    // mutex_lock(&threadlib.lock);
    // int tid = find_kernel_thread(gettid())->tid;
    // mutex_unlock(&threadlib.lock);
    // return tid;
}

int thr_join(int tid, void **statusp)
{
    lprintf("join");
    mutex_lock(&threadlib.lock);
    thread_t *thread = hashtable_get(threadlib.threads, tid);
    if (!thread) {
        mutex_unlock(&threadlib.lock);
        return -1;
    }

    if (thread->joiner_tid > 0) {
        mutex_unlock(&threadlib.lock);
        return -2;
    }

    thread->joiner_tid = thr_getid();
    lprintf("waiting");
    //wait for thread to exit
    while (!thread->exited) {
        lprintf("still no");
        mutex_unlock(&threadlib.lock);
        thr_yield(tid);
        mutex_lock(&threadlib.lock);
    }

    hashtable_remove(threadlib.threads, tid);

    mutex_unlock(&threadlib.lock);

    if (statusp)
        *statusp = thread->status;



    free(thread);

    return 0;
}

// void try_start_idle()
// {

// }

void thr_exit(void *status)
{
    lprintf("exiting");
    mutex_lock(&threadlib.lock);
    thread_t *thread = hashtable_get(threadlib.threads, thr_getid());
    if (thread != NULL) {
        //otherwise error
        thread->status = status;
        thread->exited = 1;
    }


    // find_kernel_thread(gettid())->tid = -1;
    // try_start_idle();

    // find_kernel_thread(gettid())->kernel_tid = -1;
    mutex_unlock(&threadlib.lock);
    vanish();
}


int thr_yield(int tid)
{
    lprintf("yield");
    // mutex_lock(&threadlib.lock);

    // thread_t *target = hashtable_get(threadlib.threads, tid);
    // thread_t *thread = hashtable_get(threadlib.threads, thr_getid());

    // if (target == NULL) {
    //     mutex_unlock(&threadlib.lock);
    //     return -1;
    // }


    return yield(tid);







    // if (threadlib.num_kernel_threads < THREAD_N) {
    //     kernel_thread_create(func, args, thread);
    // } else {
    //     thread->kernel_tid = threadlib.kernel_threads[choose_kernel_thread()]->kernel_tid;
    //     //perform register switch
    // }

    // mutex_unlock(&threadlib.lock);
    // return 0;
}
