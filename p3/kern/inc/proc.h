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

typedef struct regs {
    unsigned eax;
    unsigned ebx;
    unsigned ecx;
    unsigned edx;
    unsigned esi;
    unsigned edi;
    unsigned esp;
    unsigned ebp;
    unsigned eip;
    unsigned eflags;
    unsigned cr0;
    unsigned cr2;
    unsigned cr3;
    unsigned cr4;
    unsigned cs;
    unsigned ds;
    unsigned es;
    unsigned fs;
    unsigned gs;
    unsigned ss;
} regs_t;

typedef struct pcb {
    int pid;
    linklist_t threads;
} pcb_t;

typedef struct tcb {
    int tid;
    int pid;
    regs_t regs;
} tcb_t;

extern hashtable_t pcbs;
extern hashtable_t tcbs;

/* Process and thread functions */
int proc_init();
int proc_new_process();
int proc_new_thread();

#endif /* _PROC_H */
