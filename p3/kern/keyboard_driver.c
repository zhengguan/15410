/** @file keyboard_driver.c
 *  @brief Function definitions for the keyboard driver library.
 *
 *	This contains functions implementing the keyboard driver library.
 *  Implements the keyboard buffer using a circular buffer.
 *
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include "driver_core.h"

#include "circular_buffer.h"

#include <asm.h>
#include <keyhelp.h>
#include <stdio.h>

//the buffer size for the keyboard
#define BUFFER_SIZE 512

//keyboard buffer
static circ_buf *cb;

/**
 * @brief Initializes the keyboard driver.
 * @details Performs all necessary initialization for the keyboard before
 * the IDT entry is installed.
 * @return Evaluates to true iff successful
 */
int keyboard_setup()
{
	if ( !(cb = cb_create(BUFFER_SIZE)) )
		return 0;
	//success
	return 1;
}

/**
 * @brief Handles a keyboard interrupt.
 * @details Adds a scancode to the buffer. This scancode is ignored if
 * the buffer is full.
 */
void keyboard_handler()
{
	//read the scancode and enqueue it
	char scancode = inb(KEYBOARD_PORT);
	cb_enqueue(cb, scancode);

	notify_interrupt_complete();
}

/**
 * @brief Returns a key in the keyboard buffer
 * @details Processes scancodes in the keyboard buffer
 * until a key is discovered.
 * @return The first character in the keyboard buffer or -1 if there are no
 * valid characters in the buffer.
 */
int readchar()
{
	char scancode;
	//process scancodes until there is a valid character
	while (cb_dequeue(cb, &scancode)) {
		kh_type kh = process_scancode(scancode);
		if (KH_HASDATA(kh) && KH_ISMAKE(kh)){
			return KH_GETCHAR(kh);
		}
	}
	//no characters found
	return -1;
}
