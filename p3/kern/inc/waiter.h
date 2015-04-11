/** @file waiter.h
 *  @brief This file defines the prototype for the waiter struct using by
 *  various locking mechanisms.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _WAITER_H
#define _WAITER_H

#include <proc.h>
typedef struct waiter {
    tcb_t *tcb;
    int reject;
} waiter_t;

#endif /* _WAITER_H */
