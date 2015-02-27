/** @file x86/interrupts.c
 *  @brief Implementation of the "simple contract" version of the PIC module.
 *  @author de0u
 */

#include <x86/interrupt_defines.h>

void
interrupt_setup(void)
{
    /* IRQ 0..15, please occupy IDT slots 32...47 */
    pic_init(X86_PIC_MASTER_IRQ_BASE, X86_PIC_SLAVE_IRQ_BASE);
}
