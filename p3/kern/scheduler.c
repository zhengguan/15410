#include <stdlib.h>
#include <context_switch_asm.h>
#include <proc.h>
#include <syscall.h>
#include <simics.h>
#include <scheduler.h>
#include <asm.h>


linklist_t scheduler_queue;

int scheduler_init()
{
    if (linklist_init(&scheduler_queue) < 0)
    return -1;

    return 0;
}

int ctx_switch(int tid)
{
    tcb_t *new_tcb;
    if (hashtable_get(&tcbs, tid, (void**)&new_tcb) < 0)
    return -1;

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

void scheduler_tick(unsigned num_ticks)
{
      int tid;

      lprintf("tick");
      disable_interrupts();

      if (linklist_remove_head(&scheduler_queue, (void**)&tid) < 0) {
        lprintf("fucked up0");
        MAGIC_BREAK;
      }

      linklist_add_tail(&scheduler_queue, (void*)tid);

      ctx_switch(tid);
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
    
    // TODO implement this
    // yield();
    
    return 0;
}

int make_runnable(int tid) {
    if (linklist_contains(&scheduler_queue, (void *)tid)) {
        return -1;
    }
    
    linklist_add_tail(&scheduler_queue, (void*)tid);
    
    return 0;
}
