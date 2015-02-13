/** @file thread.c
 *  @brief This file implements the interface for the thread library.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <thread.h>
#include <thr_internals.h>
#include <syscall.h>
#include <stdlib.h>

extern stackinfo_t g_stackinfo;

static threadlib_t threadlib;

int thr_init(unsigned int size) {
    threadlib.stack_size = size;
    threadlib.stack_base = (void *)(g_stackinfo.stack_low + size);
    threadlib.threads = init_hashtable(HASH_TABLE_SIZE);
}

int thr_create(void *(*func)(void *), void *args) {

}

int thr_join(int tid, void **statusp) {

}

void thr_exit(void *status) {

}

int thr_getid() {
    return gettid();
}

int thr_yield(int tid) {

}
