/** @file thread.h
 *  @brief Prototypes for managing kernel threads.
 *
 *  This contains prototypes for managing kernel threads.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _THREAD_H
#define _THREAD_H

#include <linklist.h>

#define PCB_HT_SIZE 128
#define TCB_HT_SIZE 128

typedef struct pcb {
    int pid;
    linklist_t threads;
} pcb_t;

typedef struct tcb {
    int tid;
    int esp;
    int pid;
} tcb_t;

int new_process();
int new_thread();

int gettid();

#endif /* _THREAD_H */
