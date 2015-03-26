/** @file keyboard.c
 *  @brief Function definitions for the keyboard driver.
 *
 *  Implements the keyboard buffer using a circular buffer.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <driver.h>
#include <asm.h>
#include <keyhelp.h>
#include <stdio.h>
#include <macros.h>
#include <malloc.h>
#include <circbuf.h>

#define BUFFER_SIZE 512

static circbuf_t *cb;

/** @brief Initializes the keyboard driver.
 *
 *  @return 0 on success, negative error code otherwise.
 */
int keyboard_init()
{
    cb = malloc(sizeof(circbuf_t));
	if (cb == NULL) {
		return -1;
	}
	
	if (cb_init(cb, BUFFER_SIZE) < 0) {
        return -2;
	}
	
	return 0;
}

/** @brief Handles a keyboard interrupt.
 *  
 *  Adds a scancode to the buffer. This scancode is ignored if
 *  the buffer is full.
 *
 *  @return Void.
 */
void keyboard_handler()
{
	char scancode = inb(KEYBOARD_PORT);
	cb_enqueue(cb, scancode);

	notify_interrupt_complete();
}

/** @brief Returns the next character in the keyboard buffer.
 *
 *  @return The next character in the keyboard buffer, or -1 if the keyboard
 *  buffer does not currently contain a valid character.
 */
int readchar()
{
	char scancode;
	while (cb_dequeue(cb, &scancode) == 0) {
		kh_type kh = process_scancode(scancode);
		if (KH_HASDATA(kh) && KH_ISMAKE(kh)){
			return KH_GETCHAR(kh);
		}
	}
	
	return -1;
}
