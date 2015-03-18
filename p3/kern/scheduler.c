#include <context_switch_asm.h>
#include <linklist.h>
#include <proc.h>
#include <syscall.h>
#include <simics.h>
#include <scheduler.h>
#include <asm.h>

static linklist_t scheduler_queue;

int scheduler_init()
{
  if (linklist_init(&scheduler_queue) < 0)
    return -1;

  linklist_add_head(&scheduler_queue, (void*)gettid());
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
  int tid;
  if (linklist_remove_head(&scheduler_queue, (void**)&tid) < 0) {
    lprintf("fucked up");
    MAGIC_BREAK;
  }

  linklist_add_tail(&scheduler_queue, (void*)gettid());

  if (ctx_switch(tid) < 0) {
    lprintf("fucked up");
    MAGIC_BREAK;
  }
}