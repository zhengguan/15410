/** @file proc.h
 *  @brief Prototypes for managing kernel processes and threads.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _PROC_H
#define _PROC_H

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

/* Process and thread functions */
int proc_init();
int proc_new_process();
int proc_new_thread();

#endif /* _PROC_H */
