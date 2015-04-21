/** @file lib/simics.h
 *  @brief Simics interface (kernel side)
 *  @author matthewj S2008
 */

#ifndef LIB_SIMICS_H
#define LIB_SIMICS_H

#define SIM_IN_SIMICS       0x04100000
#define SIM_MEMSIZE         0x04100001
#define SIM_PUTS            0x04100002
#define SIM_BREAKPOINT      0x04100003
#define SIM_HALT            0x04100004
#define SIM_REG_PROCESS     0x04100005
#define SIM_UNREG_PROCESS   0x04100006
#define SIM_REG_CHILD       0x04100007
#define SIM_BOOTED          0x04100008
#define SIM_TID_SET         0x04100009
#define SIM_TID_DEL         0x0410000A

#ifdef ASSEMBLER

#define lprintf sim_printf

#else /* !ASSEMBLER */

/** @brief Calls Simics. Arguments are ebx, ecx, edx. */
extern int sim_call(int ebx, ...);

/** @brief Returns whether we are in simics */
extern int sim_in_simics(void);

/** @brief Returns machine memory size, from simics */
extern int sim_memsize(void);

/** @brief Prints a string to the simics console */
extern void sim_puts(const char *arg);

/** @brief Breakpoint */
extern void sim_breakpoint(void);

/** @brief Halt the simulation */
extern void sim_halt(void);

/** @brief Register a user process with the CS410 simics debugging code */
extern void sim_reg_process(void *cr3, const char *fname);

/** @brief Register a user process as a fork of another.
 *
 *  Handy if fork() is oblivious of the program's name.
 */
extern void sim_reg_child(void *child_cr3, void *parent_cr3);

/** @brief Unregister a user process from the CS410 simics debugging code */
extern void sim_unreg_process(void *cr3);

/** @brief Convenience wrapper around sprintf for simics */
extern void sim_printf(const char *fmt, ...) __attribute__((__format__ (__printf__, 1, 2)));

/** @brief Notify simics that we have booted.
 *
 *  This is done for you in 410kern/entry.c
 */
extern void sim_booted(const char *kern);

/* "Compatibility mode" for old code */
#define MAGIC_BREAK sim_breakpoint()
#define lprintf(...) sim_printf(__VA_ARGS__)

#endif /* !ASSEMBLER */

#endif /* !LIB_SIMICS_H */
