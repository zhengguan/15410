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
#include <mutex.h>
#include <vm.h>

#define KERNEL_STACK_SIZE (2 * PAGE_SIZE)

typedef struct locks {
    mutex_t new_pages;
    mutex_t remove_pages;
    memlock_t memlock;
    mutex_t malloc;
} locks_t;

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

typedef struct {
    swexn_handler_t eip;
    void *esp3;
    void *arg;
} handler_t;



typedef struct pcb {
    int pid;
    int status;
    int num_threads;
    int num_children;
    linklist_t children;
    struct pcb *parent_pcb;
    pd_t pd;

    locks_t locks;

    cond_t wait_cv;
    mutex_t vanished_procs_mutex;
    linklist_t vanished_procs;
} pcb_t;

typedef struct tcb {
    int tid;
    pcb_t *pcb;
    unsigned esp0;
    regs_t regs;
    handler_t swexn_handler;
} tcb_t;

extern hashtable_t tcbs;
extern tcb_t *cur_tcb;
extern tcb_t *idle_tcb;
extern pcb_t* init_pcb;
extern bool mt_mode;

/* Process and thread functions */
int proc_init();
int proc_new_process(pcb_t **pcb_out, tcb_t **tcb_out);
int proc_new_thread(pcb_t *pcb, tcb_t **tcb_out);
tcb_t *gettcb();
int getpid();
pcb_t *getpcb();
tcb_t *lookup_tcb(int tid);
void thread_reaper() NORETURN;
void proc_kill_thread(const char *fmt, ...) NORETURN;

#endif /* _PROC_H */
