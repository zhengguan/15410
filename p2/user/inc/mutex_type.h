/** @file mutex_type.h
 *  @brief This file defines the type for mutexes.
 */

#ifndef _MUTEX_TYPE_H
#define _MUTEX_TYPE_H


typedef struct mutex {
    int active_flag;
    int lock;
    int tid;
    int count;
    int count_lock;
} mutex_t;

#endif /* _MUTEX_TYPE_H */
