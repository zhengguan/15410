/** @file context_switch_asm.h
 *  @brief Function declaration for context_switch_asm
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */


#ifndef __CTX_SWITCH_ASM_H__
#define __CTX_SWITCH_ASM_H__

#include <proc.h>

int store_regs(regs_t *regs, unsigned old_esp0);
void restore_regs(regs_t *regs);

#endif /* __CTX_SWITCH_ASM_H__ */
