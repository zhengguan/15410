/** @file circbuf.c
 *  @brief An implementation of circular buffers.
 *
 *  The circular buffer is implemented as an array of size s+1 where s
 *  is the size of the buffer. There is one element always left empty so that
 *  we can distinguish between an empty and a full buffer.
 *  We keep a pointer to the beginning and the end of the data section of
 *  the array which shift with each insert and removal operation.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <circbuf.h>
#include <malloc.h>
#include <stdlib.h>

/** @brief Creates a new circular buffer.
 *
 *  @param size The size of the circular buffer.
 *  @return 0 on success, negative error code otherwise.
 */
int cb_init(circbuf_t *cb, int size)
{
	cb->buf = malloc(size * sizeof(elem_t));
	if (cb->buf == NULL) {
		return -1;
	}

	cb->size = size;
	cb->start = 0;
	cb->end = 0;
	
	return 0;
}

/** @brief Enqueues an element in a circular buffer
 *
 *  @param cb The circular buffer
 *  @param elem The element to add to the circular buffer.
 *  @return 0 on success, negative error code otherwise.
 */
int cb_enqueue(circbuf_t *cb, elem_t elem)
{
	if (cb == NULL || cb->buf == NULL) {
	    return -1;
	}
	
	if (cb_full(cb)) {
		return -2;
	}
    
    cb->end = INC_END(cb);
	cb->buf[cb->end] = elem;
	
	return 0;
}

/** @brief Dequeues an element from the circular buffer.
 *
 *  @param cb The circular buffer
 *  @param elem A location in memory to store the removed element.
 *  @return 0 on success, negative error code otherwise.
 */
int cb_dequeue(circbuf_t *cb, elem_t *elem)
{
	if (cb == NULL || cb->buf == NULL) {
	    return -1;
	}
	
	if (cb_empty(cb)) {
		return -2;
	}

	*elem = cb->buf[cb->start];
	cb->start = INC_START(cb);
	
	return 0;
}

/** @brief Determines if a circular buffer is full.
 *
 *  @param cb The circulr buffer.
 *  @return True if the circular buffer is full, false otherwise.
 */
bool cb_full(circbuf_t *cb)
{
	return (cb != NULL) && (cb->start == INC_END(cb));
}

/** @brief Determines if a circular buffer is empty.
 *
 *  @param cb The circulr buffer.
 *  @return True if the circular buffer is empty, false otherwise.
 */
bool cb_empty(circbuf_t *cb)
{
	return (cb != NULL) && (cb->start == cb->end);
}

/** @brief Frees a circular buffer from memory.
 *
 *  @param cb The circular buffer.
 *  @return Void.
 */
void cb_free(circbuf_t *cb)
{
	free(cb);
}
