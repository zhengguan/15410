/** @file handler.h
 *  @brief Prototypes for interrupt handlers.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _HANDLER_H
#define _HANDLER_H

/* Drivers */

void timer_handler_int();
void keyboard_int();

/* Life cycle */

int fork_int();
int exec_int(char *execname, char *argvec[]);

/* Thread management */
int gettid_int(void);
int yield_int(int tid);
int deschedule_int(int *flag);
int make_runnable_int(int tid);

/* Memory management */
int new_pages_int(void * addr, int len);
int remove_pages_int(void * addr);

/* Console I/O */

/* Miscellaneous */

/* "Special" */

#endif /* _HANDLER_H */
