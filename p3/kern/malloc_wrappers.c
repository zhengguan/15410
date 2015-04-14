#include <stddef.h>
#include <malloc.h>
#include <malloc_internal.h>
#include <malloc_wrappers.h>
#include <proc.h>

mutex_t malloc_mutex;

int malloc_init()
{
    return mutex_init(&malloc_mutex);
}

/* safe versions of malloc functions */
void *malloc(size_t size)
{
    mutex_lock(&malloc_mutex);
    void *mem = _malloc(size);
    mutex_unlock(&malloc_mutex);
    return mem;
}

void *memalign(size_t alignment, size_t size)
{
    mutex_lock(&malloc_mutex);
    void *mem = _memalign(alignment, size);
    mutex_unlock(&malloc_mutex);
    return mem;
}

void *calloc(size_t nelt, size_t eltsize)
{
    mutex_lock(&malloc_mutex);
    void *mem = _calloc(nelt, eltsize);
    mutex_unlock(&malloc_mutex);
    return mem;
}

void *realloc(void *buf, size_t new_size)
{
    mutex_lock(&malloc_mutex);
    void *mem = _realloc(buf, new_size);
    mutex_unlock(&malloc_mutex);
    return mem;
}

void free(void *buf)
{
    if (buf == NULL) {
        return;
    }
    mutex_lock(&malloc_mutex);
    _free(buf);
    mutex_unlock(&malloc_mutex);
}

void *smalloc(size_t size)
{
    mutex_lock(&malloc_mutex);
    void *mem = _smalloc(size);
    mutex_unlock(&malloc_mutex);
    return mem;
}

void *smemalign(size_t alignment, size_t size)
{
    mutex_lock(&malloc_mutex);
    void *mem = _smemalign(alignment, size);
    mutex_unlock(&malloc_mutex);
    return mem;
}

void sfree(void *buf, size_t size)
{
    if (buf == NULL) {
        return;
    }
    mutex_lock(&malloc_mutex);
    _sfree(buf, size);
    mutex_unlock(&malloc_mutex);
}


