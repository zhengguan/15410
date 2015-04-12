#include <stddef.h>
#include <malloc.h>
#include <malloc_internal.h>
#include <malloc_wrappers.h>
#include <proc.h>

/* safe versions of malloc functions */
void *malloc(size_t size)
{
    if (mt_mode) {
        mutex_lock(&getpcb()->locks.malloc);
    }
    void *mem = _malloc(size);
    if (mt_mode) {
        mutex_unlock(&getpcb()->locks.malloc);
    }
    return mem;
}

void *memalign(size_t alignment, size_t size)
{
    if (mt_mode) {
        mutex_lock(&getpcb()->locks.malloc);
    }
    void *mem = _memalign(alignment, size);
    if (mt_mode) {
        mutex_unlock(&getpcb()->locks.malloc);
    }
    return mem;
}

void *calloc(size_t nelt, size_t eltsize)
{
    if (mt_mode) {
        mutex_lock(&getpcb()->locks.malloc);
    }
    void *mem = _calloc(nelt, eltsize);
    if (mt_mode) {
        mutex_unlock(&getpcb()->locks.malloc);
    }
    return mem;
}

void *realloc(void *buf, size_t new_size)
{
    if (mt_mode) {
        mutex_lock(&getpcb()->locks.malloc);
    }
    void *mem = _realloc(buf, new_size);
    if (mt_mode) {
        mutex_unlock(&getpcb()->locks.malloc);
    }
    return mem;
}

void free(void *buf)
{
    if (buf == NULL) {
        return;
    }
    if (mt_mode) {
        mutex_lock(&getpcb()->locks.malloc);
    }
    _free(buf);
    if (mt_mode) {
        mutex_unlock(&getpcb()->locks.malloc);
    }
}

void *smalloc(size_t size)
{
    if (mt_mode) {
        mutex_lock(&getpcb()->locks.malloc);
    }
    void *mem = _smalloc(size);
    if (mt_mode) {
        mutex_unlock(&getpcb()->locks.malloc);
    }
    return mem;
}

void *smemalign(size_t alignment, size_t size)
{
    if (mt_mode) {
        mutex_lock(&getpcb()->locks.malloc);
    }
    void *mem = _smemalign(alignment, size);
    if (mt_mode) {
        mutex_unlock(&getpcb()->locks.malloc);
    }
    return mem;
}

void sfree(void *buf, size_t size)
{
    if (buf == NULL) {
        return;
    }
    if (mt_mode) {
        mutex_lock(&getpcb()->locks.malloc);
    }
    _sfree(buf, size);
    if (mt_mode) {
        mutex_unlock(&getpcb()->locks.malloc);
    }
}


