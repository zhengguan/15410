/** @file timer.c
 *  @brief Function definitions for the timer driver.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <driver.h>
#include <timer.h>
#include <stdlib.h>
#include <x86/asm.h>
#include <x86/timer_defines.h>
#include <scheduler.h>
#include <syscall.h>

#define TICKS_PER_SECOND 1
#define CYCLES_BTW_INTER ((unsigned)(TIMER_RATE / TICKS_PER_SECOND))

static unsigned ticks = 0;

/** @brief Initializes the timer driver.
 *
 *  @return 0 on success, negative error code otherwise.
 */
int timer_init()
{
	outb(TIMER_MODE_IO_PORT, TIMER_SQUARE_WAVE);
	outb(TIMER_PERIOD_IO_PORT, CYCLES_BTW_INTER & 0xFF);
	outb(TIMER_PERIOD_IO_PORT, (CYCLES_BTW_INTER >> 8) & 0xFF);
	
	return 0;
}

/** @brief Wrapper for the stored tick_callback.
 *
 *  @return Void.
 */
void timer_handler()
{
	ticks++;
	
	scheduler_tick(ticks);
	
	notify_interrupt_complete();
}

unsigned get_ticks()
{
    return ticks;
}
