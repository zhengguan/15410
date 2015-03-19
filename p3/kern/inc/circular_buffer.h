/** @file circular_buffer.h
 *  @brief An interface for the circular buffer library
 *
 *  This contains the interface for a circular buffer.
 *  It is an implementation of a queue with all constant time operations,
 *  but with a fixed maximum size defined on creation.
 *
 *  There is undefined behavior if the circular buffer passed in any function
 *  is not a valid circular buffer.
 *
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef __CIRC_BUF_H_
#define __CIRC_BUF_H_

//We use chars as elements of the buffer. This can be changed.
typedef char elem_t;

typedef struct circ_buf circ_buf;


circ_buf *cb_create(unsigned size);

int cb_full(const circ_buf *cb);

int cb_empty(const circ_buf *cb);

int cb_enqueue(circ_buf *cb, elem_t elem);

int cb_dequeue(circ_buf *cb, elem_t *elem);

void cb_free(circ_buf *cb);

#endif /* __CIRC_BUF_H_ */

