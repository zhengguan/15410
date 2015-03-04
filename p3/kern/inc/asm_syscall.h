/* The 15-410 kernel.
 * asm_syscall.h
 *
 * Prototypes for the user-land C library interface
 * to system calls.
 *
 */

#ifndef _ASM_SYSCALL_H
#define _ASM_SYSCALL_H

#define NORETURN __attribute__((__noreturn__))

#define PAGE_SIZE 0x0001000 /* 4096 */

/* Life cycle */
int asm_exec(char *execname, char *argvec[]);

/* Thread management */
int asm_gettid(void);

/* Memory management */
int asm_new_pages(void * addr, int len);
int asm_remove_pages(void * addr);

/* Console I/O */

/* Miscellaneous */

/* "Special" */

#endif /* _ASM_SYSCALL_H */
