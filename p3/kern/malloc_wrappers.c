#include <stddef.h>
#include <malloc.h>
#include <malloc_internal.h>
#include <malloc_wrappers.h>
#include <proc.h>

/* safe versions of malloc functions */
void *malloc(size_t size)
{
    // TODO make this less ugly
    int tid = gettid();
    if (tid != 0) {
        mutex_lock(&(*CUR_PCB)->locks.malloc);
    }
    void *mem = _malloc(size);
    if (tid != 0) {
        mutex_unlock(&(*CUR_PCB)->locks.malloc);
    }
    return mem;
}

void *memalign(size_t alignment, size_t size)
{
    mutex_lock(&(*CUR_PCB)->locks.malloc);
    void *mem = _memalign(alignment, size);
    mutex_unlock(&(*CUR_PCB)->locks.malloc);
    return mem;
}

void *calloc(size_t nelt, size_t eltsize)
{
    mutex_lock(&(*CUR_PCB)->locks.malloc);
    void *mem = _calloc(nelt, eltsize);
    mutex_unlock(&(*CUR_PCB)->locks.malloc);
    return mem;
}

void *realloc(void *buf, size_t new_size)
{
    mutex_lock(&(*CUR_PCB)->locks.malloc);
    void *mem = _realloc(buf, new_size);
    mutex_unlock(&(*CUR_PCB)->locks.malloc);
    return mem;
}

void free(void *buf)
{
    if (buf == NULL) {
        return;
    }
    mutex_lock(&(*CUR_PCB)->locks.malloc);
    _free(buf);
    mutex_unlock(&(*CUR_PCB)->locks.malloc);
}

void *smalloc(size_t size)
{
    mutex_lock(&(*CUR_PCB)->locks.malloc);
    void *mem = _smalloc(size);
    mutex_unlock(&(*CUR_PCB)->locks.malloc);
    return mem;
}

void *smemalign(size_t alignment, size_t size)
{
    int tid = gettid();
    if (tid != 0) {
        mutex_lock(&(*CUR_PCB)->locks.malloc);
    }
    void *mem = _smemalign(alignment, size);
    if (tid != 0) {
        mutex_unlock(&(*CUR_PCB)->locks.malloc);
    }
    return mem;
}

void sfree(void *buf, size_t size)
{
    if (buf == NULL) {
        return;
    }
    mutex_lock(&(*CUR_PCB)->locks.malloc);
    _sfree(buf, size);
    mutex_unlock(&(*CUR_PCB)->locks.malloc);
}


