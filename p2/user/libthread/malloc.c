/*
 * these functions should be thread safe.
 * It is up to you to rewrite them
 * to make them thread safe.
 *
 */

#include <stdlib.h>
#include <types.h>
#include <stddef.h>
#include <mutex.h>

static mutex_t mutex;

void *malloc(size_t __size)
{
	mutex_lock(&mutex);
	void *mem = _malloc(__size);
	mutex_unlock(&mutex);
  return mem;
}

void *calloc(size_t __nelt, size_t __eltsize)
{
  mutex_lock(&mutex);
	void *mem = _calloc(__nelt, __eltsize);
	mutex_unlock(&mutex);
  return mem;
}

void *realloc(void *__buf, size_t __new_size)
{
  mutex_lock(&mutex);
	void *mem = _realloc(__buf, __new_size);
	mutex_unlock(&mutex);
  return mem;
}

void free(void *__buf)
{
 	mutex_lock(&mutex);
	_free(__buf);
	mutex_unlock(&mutex);
}
