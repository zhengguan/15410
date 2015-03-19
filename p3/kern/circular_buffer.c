/** @file circular_buffer.c
 *  @brief Function definitions for the circular buffer library.
 *
 *  This contains the functions definitions for the circular buffer library.
 *  The circular buffer is implemented as an array of size s+1 where s
 *  is the size of the buffer. There is one element always left empty so that
 *  we can distinguish between an empty and a full buffer.
 *  We keep a pointer to the beginning and the end of the data section of
 *  the array which shift with each insert and removal operation.
 *
 *
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
#include "circular_buffer.h"

#include <malloc.h>
#include <stdlib.h>

//The structural implementation of the circular buffer
struct circ_buf {
	unsigned size;
	elem_t *start; //points to the first data element
	elem_t *end;   //points to one element after the last data element
	elem_t buf[];
};

/**
 * @brief Creates a new circular buffer
 * @param size The size of the circular buffer to create
 * @return A pointer to the newly created circular buffer. Null on failure.
 */
circ_buf *cb_create(unsigned size)
{
	circ_buf *cb;
	if (!(cb = malloc(sizeof(circ_buf) + sizeof(elem_t)*(size+1))))
		return NULL;

	cb->size = size;
	cb->start = cb->buf;
	cb->end = cb->buf;
	return cb;
}

/**
 * @brief Determines whether a circular buffer is full
 * @param cb A pointer to the circulr buffer
 * @return Evaluates to true iff the circular buffer is full
 */
int cb_full(const circ_buf *cb)
{
	return (cb->start == cb->end+1 ||
				 (cb->start == cb->buf && cb->end == cb->buf + cb->size - 1));
}

/**
 * @brief Determines whether a circular buffer is empty
 * @param cb A pointer to the circular buffer
 * @return Evaluates to true iff the circular buffer is empty
 */
int cb_empty(const circ_buf *cb)
{
	return (cb->start == cb->end);
}

/**
 * @brief Helper function that increment a pointer one element
 * in the circular buffer
 * @details Undefined behavior if ptr does not point to an element in the
 * buffer
 *
 * @param cb A pointer to the circular buffer
 * @param ptr A pointer to some element in the buf array in cb
 */
static void inc_ptr(circ_buf *cb, elem_t **ptr)
{
	//if at end wrap around to beginning
	if (*ptr == cb->buf + cb->size - 1)
			*ptr = cb->buf;
	//typically, just increment
	else
		(*ptr)++;
}

/**
 * @brief Adds an element to a circular buffer
 * @details If the circular buffer is full, the circular buffer is unchanged.
 * Otherwise, the element is added to the buffer
 *
 * @param cb A pointer to the circular buffer
 * @param elem The element to add to the circular buffer
 * @return Evaluates to true iff the element was successfully
 * enqueued into the buffer
 */
int cb_enqueue(circ_buf *cb, elem_t elem)
{
	if (cb_full(cb))
		return 0;

	*cb->end = elem;
	inc_ptr(cb, &cb->end);
	return 1;
}

/**
 * @brief Dequeues an element from the circular buffer
 * @details If the circular buffer is not empty, an element is removed from
 * the circular buffer
 *
 * @param cb A pointer to the circular buffer
 * @param elem A pointer at which the element returned will be stored.
 *
 * @return Evaluates to true iff an element was successfully
 * dequeued from the buffer
 */
int cb_dequeue(circ_buf *cb, elem_t *elem)
{
	if (cb_empty(cb))
		return 0;

	*elem = *cb->start;
	inc_ptr(cb, &cb->start);
	return 1;
}

/**
 * @brief Frees a circular buffer from memory
 * @param cb The circular buffer to free
 */
void cb_free(circ_buf *cb)
{
	free(cb);
}