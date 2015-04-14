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
#include <rwlock.h>

#define KERNEL_STACK_SIZE (2 * PAGE_SIZE)

/* Process specific locks */
typedef struct locks {
    mutex_t new_pages;
    rwlock_t remove_pages;
    memlock_t memlock;
} locks_t;

/* Register struct for context switching */
typedef struct regs {
    unsigned ebx;           // 0
    unsigned esi;           // 4
    unsigned edi;           // 8
    unsigned esp_offset;    // 12
    unsigned ebp_offset;    // 16
    unsigned eip;           // 20
    unsigned eflags;        // 24
    unsigned cr2;           // 28
    unsigned cr3;           // 32
} regs_t;

/* Swexn handler */
typedef struct {
    swexn_handler_t eip;
    void *esp3;
    void *arg;
} handler_t;

/* Process control block */
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
    mutex_t proc_mutex;
    linklist_t vanished_procs;
    listnode_t pcb_listnode;
} pcb_t;

/* Thread control block */
typedef struct tcb {
    int tid;
    pcb_t *pcb;
    unsigned esp0;
    regs_t regs;
    handler_t swexn_handler;
    listnode_t scheduler_listnode;
    int sleep_flag;
    bool user_descheduled;
} tcb_t;

extern tcb_t *cur_tcb;
extern tcb_t *idle_tcb;
extern pcb_t* init_pcb;

/* Process and thread functions */
int proc_init();
int proc_new_process(pcb_t **pcb_out, tcb_t **tcb_out);
int proc_new_thread(pcb_t *pcb, tcb_t **tcb_out);
tcb_t *gettcb();
int getpid();
pcb_t *getpcb();
tcb_t *lookup_tcb(int tid);
void proc_kill_thread(const char *fmt, ...) NORETURN;
int reap_pcb(pcb_t *pcb, int *status_ptr);

void thread_reaper() NORETURN;

#endif /* _PROC_H */
