/** @file context_switch_asm.h
 *  @brief Function prototypes for context switching.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */


#ifndef _CONTEXT_SWITCH_H
#define _CONTEXT_SWITCH_H

#include <proc.h>

int store_regs(regs_t *regs, unsigned cur_esp0);
void restore_regs(regs_t *regs, unsigned new_esp0) NORETURN;

#endif /* _CONTEXT_SWITCH_H */
