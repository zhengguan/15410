/** @file scheduler.h
 *  @brief Function declarations for scheduler
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef __SCHEDULER_INIT_H__
#define __SCHEDULER_INIT_H__

int scheduler_init();
void scheduler_tick(unsigned num_ticks);
int ctx_switch(int tid);

#endif /* __SCHEDULER_INIT_H__ */