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

#define PCB_HT_SIZE 128
#define TCB_HT_SIZE 128

#define KERNEL_STACK_SIZE (2 * PAGE_SIZE)

typedef struct regs {
    unsigned ebx;           // 0
    unsigned esi;           // 4
    unsigned edi;           // 8
    unsigned esp_offset;    // 12
    unsigned ebp;           // 16
    unsigned eip;           // 20
    unsigned eflags;        // 24
    unsigned cr0;           // 28
    unsigned cr2;           // 32
    unsigned cr3;           // 36
    unsigned cr4;           // 40
    unsigned cs;            // 44
    unsigned ds;            // 48
    unsigned es;            // 52
    unsigned fs;            // 56
    unsigned gs;            // 60
    unsigned ss;            // 64
    unsigned esp0;          // 68
} regs_t;

typedef struct pcb {
    int pid;
    linklist_t threads;
} pcb_t;

typedef struct tcb {
    int tid;
    int pid;
    int status;
    regs_t regs;
} tcb_t;

extern hashtable_t pcbs;
extern hashtable_t tcbs;
extern int cur_tid;

/* Process and thread functions */
int proc_init();
int proc_new_process();
int proc_new_thread();

#endif /* _PROC_H */
