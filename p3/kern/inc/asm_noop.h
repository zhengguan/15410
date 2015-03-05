/* The 15-410 kernel.
 * asm_syscall.h
 *
 * Prototypes for the user-land C library interface
 * to system calls.
 *
 */

#ifndef _ASM_NOOP_H
#define _ASM_NOOP_H

#define NORETURN __attribute__((__noreturn__))

#define PAGE_SIZE 0x0001000 /* 4096 */

void asm_noop();

#endif /* _ASM_NOOP_H */
