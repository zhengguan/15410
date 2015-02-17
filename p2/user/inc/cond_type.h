/** @file cond_type.h
 *  @brief This file defines the type for condition variables.
 */

#ifndef _COND_TYPE_H
#define _COND_TYPE_H

#include <linklist.h>

typedef struct cond {
    int valid;
    linklist_t *queue;
    mutex_t *mutex;
} cond_t;

#endif /* _COND_TYPE_H */
