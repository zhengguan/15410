#include <stddef.h>
#include <malloc.h>
#include <malloc_internal.h>
#include <malloc_wrappers.h>
#include <proc.h>

/* safe versions of malloc functions */
void *malloc(size_t size)
{
    proc_lock(MALLOC);
    void *mem = _malloc(size);
    proc_unlock(MALLOC);
    return mem;
}

void *memalign(size_t alignment, size_t size)
{
    proc_lock(MALLOC);
    void *mem = _memalign(alignment, size);
    proc_unlock(MALLOC);
    return mem;
}

void *calloc(size_t nelt, size_t eltsize)
{
    proc_lock(MALLOC);
    void *mem = _calloc(nelt, eltsize);
    proc_unlock(MALLOC);
    return mem;
}

void *realloc(void *buf, size_t new_size)
{
    proc_lock(MALLOC);
    void *mem = _realloc(buf, new_size);
    proc_unlock(MALLOC);
    return mem;
}

void free(void *buf)
{
    if (buf == NULL)
        return;
    proc_lock(MALLOC);
    _free(buf);
    proc_unlock(MALLOC);
}

void *smalloc(size_t size)
{
    proc_lock(MALLOC);
    void *mem = _smalloc(size);
    proc_unlock(MALLOC);
    return mem;
}

void *smemalign(size_t alignment, size_t size)
{
    proc_lock(MALLOC);
    void *mem = _smemalign(alignment, size);
    proc_unlock(MALLOC);
    return mem;
}

void sfree(void *buf, size_t size)
{
    if (buf == NULL)
        return;
    proc_lock(MALLOC);
    _sfree(buf, size);
    proc_unlock(MALLOC);
}


