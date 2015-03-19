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


  disable_interrupts();
  if (store_registers_asm(old_tcb)) {
    enable_interrupts();
    return;
  }
  else {
    context_switch_asm(new_tcb);
  }

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

  ctx_switch(tid);
}