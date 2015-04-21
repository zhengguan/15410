/** @file malloc.c
 *  @brief Thread safe wrappers for memory allocation library functions.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <mutex.h>
#include <stddef.h>
#include <stdlib.h>
#include <types.h>

mutex_t malloc_mutex = {
    .valid = 1,
    .lock = 0,
    .tid = -1
};

void *malloc(size_t __size)
{
    mutex_lock(&malloc_mutex);
    void *mem = _malloc(__size);
    mutex_unlock(&malloc_mutex);
    return mem;
}

void *calloc(size_t __nelt, size_t __eltsize)
{
    mutex_lock(&malloc_mutex);
    void *mem = _calloc(__nelt, __eltsize);
    mutex_unlock(&malloc_mutex);
    return mem;
}

void *realloc(void *__buf, size_t __new_size)
{
    mutex_lock(&malloc_mutex);
    void *mem = _realloc(__buf, __new_size);
    mutex_unlock(&malloc_mutex);
    return mem;
}

void free(void *__buf)
{
    mutex_lock(&malloc_mutex);
    _free(__buf);
    mutex_unlock(&malloc_mutex);
}
