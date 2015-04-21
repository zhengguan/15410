/** @file x86/interrupt_defines.h
 *  @brief The "simple contract" version of the PIC module.
 *  @author de0u
 */

#ifndef _INTERRUPT_DEFINES_H_
#define _INTERRUPT_DEFINES_H_

/* import "everything" from our backer */
#define X86_PIC_DEFINITIONS
#include <x86/pic.h>

#define INT_CTL_PORT     (MASTER_ICW)
#define INT_ACK_CURRENT  (NON_SPEC_EOI)

#ifndef ASSEMBLER
void interrupt_setup(void);
#endif /* ASSEMBLER */

#endif /* !_INTERRUPT_DEFINES_H_ */
