/** @file timer_driver.c
 *  @brief Function definitions for the timer driver library.
 *
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include "driver_core.h"
#include "timer_driver.h"

#include <stdlib.h>
#include <x86/asm.h>
#include <x86/timer_defines.h>

#define TICKS_PER_SECOND 100
#define CYCLES_BTW_INTER ((unsigned)(TIMER_RATE / TICKS_PER_SECOND))

static unsigned ticks = 0;

/**
 * @brief Pointer to void (*) (unsigned) callback function
 * @details This is called by a timer tick once assigned.
 */
void (*tick_callback)(unsigned) = NULL;


/**
 * @brief Initializes the timer driver
 * @details Performs all necessary initialization for the timer before
 * the IDT entry is installed.
 * @return Evaluates to true iff successful
 */
int timer_setup()
{
	outb(TIMER_MODE_IO_PORT, TIMER_SQUARE_WAVE);
	outb(TIMER_PERIOD_IO_PORT, CYCLES_BTW_INTER & 0xFF);
	outb(TIMER_PERIOD_IO_PORT, (CYCLES_BTW_INTER >> 8) & 0xFF);
	return 1;
}

/**
 * @brief Wrapper for the stored tick_callback.
 */
void timer_handler()
{
	ticks++;
	if (tick_callback)
		tick_callback(ticks);
	notify_interrupt_complete();
}