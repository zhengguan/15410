/** @file context_switch_asm.h
 *  @brief Function declaration for context_switch_asm
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */


#ifndef _CONTEXT_SWITCH_H
#define _CONTEXT_SWITCH_H

#include <proc.h>

int store_regs(regs_t *regs, unsigned old_esp0);
void restore_regs(regs_t *regs);

#endif /* _CTX_SWITCH_ASM_H */
