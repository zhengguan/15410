/** @file sem_type.h
 *  @brief This file defines the type for semaphores.
 */

#ifndef _SEM_TYPE_H
#define _SEM_TYPE_H

#include <mutex.h>
#include <cond.h>

typedef struct sem {
	int valid;
  int count;
  mutex_t count_mutex;
  cond_t cond;
} sem_t;

#endif /* _SEM_TYPE_H */
