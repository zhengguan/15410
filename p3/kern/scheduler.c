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
  tcb_t *tcb;
  if (hashtable_get(&tcbs, tid, (void**)&tcb) < 0)
    return -1;

  context_switch_asm(tcb->esp);

  enable_interrupts();
  return 0;
}

void scheduler_tick(unsigned num_ticks)
{
  lprintf("tick");
  int tid;
  if (linklist_remove_head(&scheduler_queue, (void**)&tid) < 0) {
    lprintf("fucked up0");
    MAGIC_BREAK;
  }

  linklist_add_tail(&scheduler_queue, (void*)gettid());

  if (ctx_switch(tid) < 0) {
    lprintf("fucked up1");
    MAGIC_BREAK;
  }
}