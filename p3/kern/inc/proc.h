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
#include <hashtable.h>
#include <cond.h>
#include <kern_common.h>

#define PCB_HT_SIZE 128
#define TCB_HT_SIZE 128

#define KERNEL_STACK_SIZE (2 * PAGE_SIZE)

typedef struct regs {
    unsigned ebx;           // 0
    unsigned esi;           // 4
    unsigned edi;           // 8
    unsigned esp_offset;    // 12
    unsigned ebp_offset;    // 16
    unsigned eip;           // 20
    unsigned eflags;        // 24
    unsigned cr0;           // 28
    unsigned cr2;           // 32
    unsigned cr3;           // 36
    unsigned cr4;           // 40
} regs_t;

typedef struct pcb {
    int pid;
    int status;
    int num_threads;
    int num_children;
    int parent_pid;
    int first_tid;
    linklist_t threads;

    cond_t waiter_cv;
    mutex_t vanished_task_mutex;
    linklist_t vanished_tasks;
} pcb_t;

typedef struct tcb {
    int tid;
    int pid;
    unsigned esp0;
    regs_t regs;
} tcb_t;

extern hashtable_t pcbs;
extern hashtable_t tcbs;
extern int cur_tid;
extern int idle_tid;
extern int init_tid;

/* Process and thread functions */
int proc_init();
int proc_new_process(pcb_t **pcb_out, tcb_t **tcb_out);
int proc_new_thread(pcb_t *pcb, tcb_t **tcb_out);
int getpid();
void thread_reaper();

#endif /* _PROC_H */
