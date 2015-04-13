#ifndef _MALLOC_WRAPPERS_H
#define _MALLOC_WRAPPERS_H

#include <mutex.h>

int malloc_init();
extern mutex_t malloc_mutex;

#endif /* _MALLOC_WRAPPERS_H */
