/** @file keyboard.c
 *  @brief Function definitions for the keyboard driver.
 *
 *  Implements the keyboard buffer using a circular buffer.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <x86/video_defines.h>
#include <driver.h>
#include <asm.h>
#include <keyhelp.h>
#include <stdio.h>
#include <kern_common.h>
#include <malloc.h>
#include <circbuf.h>
#include <mutex.h>
#include <cond.h>
#include <console.h>
#include <vm.h>
#include <simics.h>

#define BUFFER_SIZE 512

#define MAX_READLINE (CONSOLE_HEIGHT * CONSOLE_WIDTH)

static circbuf_t *cb;

static mutex_t readchar_mutex;
static cond_t readchar_cond;

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

	if (mutex_init(&readchar_mutex) < 0) {
	    return -3;
	}

	if (cond_init(&readchar_cond) < 0) {
	    return -3;
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

    cond_signal(&readchar_cond);

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

/** @brief A blocking version of readchar().  Returns the next character in the
 *  keyboard buffer.
 *
 *  @return The next character in the keyboard buffer.
 */
int readchar_blocking()
{
    char c;
    while((c = readchar()) < 0) {
        cond_wait(&readchar_cond, NULL);
    }

    return c;
}

/** @brief Returns a single character from the character input stream.
 *
 *  If the input stream is empty the thread is descheduled until a character is
 *  available.  If some other thread is descheduled on readline() or getchar(),
 *  then the calling thread must block and wait its turn to access the input
 *  stream.  Characters processed by the getchar() system call should not be
 *  echoed to the console.
 *
 *  @return The next character from the character input stream.
 */
char getchar()
{
    // TODO add mutex for readline and getchar (remember to remove mutex above if not needed!)

    char c = readchar_blocking();

    return c;
}

int readline(int len, char *buf)
{
    if (!vm_is_present_len(buf, len)) {
        lprintf("return -1");
        return -1;
    }

    // TODO check if buf falls in read only memory region

    if (len > MAX_READLINE) {
        lprintf("return -3");
        return -3;
    }

    char *tmp_buf = malloc(len * sizeof(char));
    if (tmp_buf == NULL) {
        lprintf("return -4");
        return -4;
    }

    int i;
    for (i = 0; i < len; i++) {
        char c = readchar_blocking();
        tmp_buf[i] = c;
        putbyte(c);
        if (c == '\n') {
            i++;
            break;
        }
    }

    memcpy(buf, tmp_buf, i);
    return i;
}
