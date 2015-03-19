/** @file driver_core.c
 *  @brief Function definitions core driver library functions.
 *
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
#include "driver_core.h"

#include <x86/asm.h>
#include <x86/interrupt_defines.h>

/**
 * @brief Notifies the PIC that an interrupt has been processed
 */
void notify_interrupt_complete()
{
	outb(INT_CTL_PORT, INT_ACK_CURRENT);
}
