#include <stdlib.h>
#include <context_switch.h>
#include <proc.h>
#include <syscall.h>
#include <simics.h>
#include <scheduler.h>
#include <asm.h>

linklist_t scheduler_queue;

int scheduler_init()
{
    if (linklist_init(&scheduler_queue) < 0) {
        return -1;
    }

    return 0;
}

void scheduler_tick(unsigned num_ticks)
{
    disable_interrupts();
    
    int tid;
    linklist_remove_head(&scheduler_queue, (void**)&tid);
    linklist_add_tail(&scheduler_queue, (void*)tid);

    context_switch(tid);
    
    enable_interrupts();
}

int context_switch(int tid)
{
    if (tid == cur_tid) {
        return 0;
    }
    
    tcb_t *new_tcb;
    if (hashtable_get(&tcbs, tid, (void**)&new_tcb) < 0) {
        return -1;
    }

    tcb_t *old_tcb;
    hashtable_get(&tcbs, gettid(), (void**)&old_tcb);

    if (store_regs(&old_tcb->regs)) {
        cur_tid = new_tcb->tid;
        restore_regs(&new_tcb->regs);
    } else {
        enable_interrupts();
    }

    return 0;
}

// TODO Add atomicity with make runnable, use either a spinlock or disable interruptss
int deschedule(int *flag) {
    if (flag == NULL) {
        return -1;
    }

    if (*flag) {
        return 0;
    }

    linklist_remove(&scheduler_queue, (void *)gettid());
    
    yield(-1);
    
    return 0;
}

int make_runnable(int tid) {
    if (linklist_contains(&scheduler_queue, (void *)tid)) {
        return -1;
    }

    linklist_add_tail(&scheduler_queue, (void*)tid);

    return 0;
}

int yield(int tid) {
    if (tid == -1) {
        linklist_remove_head(&scheduler_queue, (void**)&tid);
    } else {
        if (linklist_remove(&scheduler_queue, (void *)tid) < 0) {
            return -1;
        }
    }
    
    linklist_add_tail(&scheduler_queue, (void*)tid);
    
    context_switch(tid);
    
    return 0;
}
