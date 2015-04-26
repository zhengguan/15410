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
#include <x86/pic.h>
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

#define BUFFER_SIZE 512

#define MAX_READLINE (CONSOLE_HEIGHT * CONSOLE_WIDTH)

#define KEY_IRQ 0x1

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

    pic_acknowledge(KEY_IRQ);
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

int readline(int len, char *buf)
{
    if (len > MAX_READLINE) {
        return -3;
    }

    int tmp_buf_size = MAX_READLINE;
    char *tmp_buf = malloc(tmp_buf_size * sizeof(char));
    if (tmp_buf == NULL) {
        return -4;
    }

    int read_len = 0;
    while (1) {
        if (read_len > tmp_buf_size) {
            tmp_buf_size *= 2;
            realloc(tmp_buf, tmp_buf_size);
        }

        char c;
        while((c = readchar()) < 0) {
            cond_wait(&readchar_cond, NULL);
        }

        if (c == '\b') {
            if (read_len > 0) {
                putbyte(c);
                read_len--;
            }
        } else {
            putbyte(c);
            tmp_buf[read_len] = c;
            read_len++;
            if (c == '\n') {
                break;
            }
        }
    }

    read_len = MIN(read_len, len);
    memcpy(buf, tmp_buf, read_len);

    return read_len;
}
