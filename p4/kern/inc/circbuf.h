/** @file circbuf.h
 *  @brief This file defines the type and function prototypes for circular
 *  buffers.
 *
 *  It is an implementation of a queue with all constant time operations,
 *  but with a fixed maximum size defined on creation.
 *
 *  There is undefined behavior if the circular buffer passed in any function
 *  is not a valid circular buffer.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _CIRCBUF_H
#define _CIRCBUF_H

#include <kern_common.h>

#define INC_START(cb) ((cb->start + 1) % cb->size)
#define INC_END(cb) ((cb->end + 1) % cb->size)

typedef char elem_t;

typedef struct circbuf {
	int size;
	int start;
	int end;
	elem_t *buf;
} circbuf_t;

/* circular buffer functions */
int cb_init(circbuf_t *cb, int size);
int cb_enqueue(circbuf_t *cb, elem_t elem);
int cb_dequeue(circbuf_t *cb, elem_t *elem);
bool cb_full(circbuf_t *cb);
bool cb_empty(circbuf_t *cb);
void cb_free(circbuf_t *cb);

#endif /* _CIRCBUF_H */

