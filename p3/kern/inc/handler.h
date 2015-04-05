/** @file handler.h
 *  @brief Prototypes for interrupt handlers.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _HANDLER_H
#define _HANDLER_H

#include <ureg.h>
#include <syscall.h>

/* Drivers */
void timer_handler_int();
void keyboard_int();

/* Life cycle */
int fork_int();
int thread_fork_int();
int exec_int(char *execname, char *argvec[]);
void set_status_int(int status);

/* Thread management */
int gettid_int(void);
int yield_int(int tid);
int deschedule_int(int *flag);
int make_runnable_int(int tid);
unsigned get_ticks_int(void);
int sleep_int(int ticks);
void swexn_int(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg);

/* Memory management */
int new_pages_int(void * addr, int len);
int remove_pages_int(void * addr);

/* Console I/O */
int readline_int(int size, char *buf);
int print_int(int size, char *buf);
int set_term_color_int(int color);
int set_cursor_pos_int(int row, int col);
int get_cursor_pos_int(int *row, int *col);

/* Miscellaneous */
void halt_int();
int readfile_int(const char *filename, char *buf, int count, int offset);

/* "Special" */
void exn_handler_complete_int();
void misbehave_int(int mode);

#endif /* _HANDLER_H */
