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
void timer_handler_int();
void keyboard_int();
int fork_int();
int exec_int(char *execname, char *argvec[]);

/* Console I/O */

/* Miscellaneous */

/* "Special" */

#endif /* _HANDLER_H */
