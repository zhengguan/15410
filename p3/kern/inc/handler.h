/** @file handler.h
 *  @brief Prototypes for interrupt handlers.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _HANDLER_H
#define _HANDLER_H

/* Life cycle */

/* Thread management */
int gettid_int(void);

/* Memory management */
int new_pages_int(void * addr, int len);
int remove_pages_int(void * addr);
void scheduler_tick_int();
int fork_int();


/* Console I/O */

/* Miscellaneous */

/* "Special" */

#endif /* _HANDLER_H */
