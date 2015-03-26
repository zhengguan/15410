/** @file driver.c
 *  @brief Function definitions for core driver functions.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include "driver.h"
#include <x86/asm.h>
#include <x86/interrupt_defines.h>

/**
 * @brief Notifies the PIC that an interrupt has been processed.
 *
 * @return Void.
 */
void notify_interrupt_complete()
{
	outb(INT_CTL_PORT, INT_ACK_CURRENT);
}
