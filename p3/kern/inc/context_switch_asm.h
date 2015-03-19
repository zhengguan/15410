/** @file context_switch_asm.h
 *  @brief Function declaration for context_switch_asm
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */


#ifndef __CTX_SWITCH_ASM_H__
#define __CTX_SWITCH_ASM_H__

void store_registers_asm(regs_t *regs);
void context_switch_asm(regs_t *regs);

#endif /* __CTX_SWITCH_ASM_H__ */