/** @file mutex.c
 *  @brief This file implements the interface for mutexes.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include <mutex.h>
#include <syscall.h>
#include <stdlib.h>

extern int atom_xchg(int *dest, int src);

int mutex_init(mutex_t *mp) {
    if (mp == NULL) {
        return -1;
    }

    mp->lock = 0;
    mp->tid = -1;
    mp->count = 0;
    mp->count_lock = 0;

    mp->active_flag = 1;
    
    return 0;
}

void mutex_destroy(mutex_t *mp) {
    if (mp == NULL) {
        return;
    }
    
    mp->active_flag = 0;
    
    while (mp->count > 0) {
        yield(mp->tid);
    }
}

void mutex_lock(mutex_t *mp) {
    if (mp == NULL) {
        return;
    }
    
    if (mp->active_flag == 0) {
        return;
    }

    while (atom_xchg(&mp->count_lock, 1) != 0) {
        yield(-1);
    }
    mp->count++;
    mp->count_lock = 0;
    
    while (atom_xchg(&mp->lock, 1) != 0) {
        yield(mp->tid);
    }
    
    mp->tid = gettid();
}

void mutex_unlock(mutex_t *mp) {
    if (mp == NULL) {
        return;
    }

    if (mp->count > 0) {
        while (atom_xchg(&mp->count_lock, 1) != 0) {
            yield(-1);
        }
        mp->count--;
        mp->count_lock = 0;
    }
    
    mp->lock = 0;
}
