/** @file cond.c
 *  @brief This file implements the interface for condition variables.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include <mutex.h>
#include <cond.h>
#include <syscall.h>
#include <stdlib.h>

/** @brief Initializes a condition variable.
 *
 *  @param list Condition variable.
 *  @return 0 on success, number less than 0 on error.
 */
int cond_init(cond_t *cv) {
    if (cv == NULL) {
        return -1;
    }  
      
    if (linklist_init(cv->queue) < 0) {
        return -1;
    }
    
    if (mutex_init(cv->mutex) < 0) {
        return -1;
    }
    
    cv->active_flag = 1;
    return 0;
}

/** @brief "Deactivates" a condition variable.
 *
 *  @param list Condition variable.
 *  @return Void.
 */
void cond_destroy(cond_t *cv) {
    if (cv == NULL) {
        return;
    }
    
    cv->active_flag = 0;
    
    while (1) {
        mutex_lock(cv->mutex);
        
        if (cv->queue->head == NULL) {
            mutex_unlock(cv->mutex);
            mutex_destroy(cv->mutex);
            return;
        }
        
        mutex_unlock(cv->mutex);
        yield(-1);        
    }
}

/** @brief Allows a thread to wait for a condition variable and release the
 *  associated mutex.
 *
 *  @param list Condition variable.
 *  @param mp Associated mutex.
 *  @return Void.
 */
void cond_wait(cond_t *cv, mutex_t *mp) {
    if (cv == NULL) {
        return;
    }
    
    if (!cv->active_flag) {
        return;
    }
    
    listnode_t *node = (listnode_t *)malloc(sizeof(listnode_t));
    node->data = (void *)gettid();
    node->next = NULL;
    
    mutex_lock(cv->mutex);
    linklist_add_tail(cv->queue, node);
    mutex_unlock(cv->mutex);
    
    mutex_unlock(mp);
    
    int reject = 0;
    deschedule(&reject); 
    
    mutex_lock(mp);  
}

/** @brief Wakes up a thread waiting on a condition variable.
 *
 *  @param list Condition variable.
 *  @return Void.
 */
void cond_signal(cond_t *cv) {
    if (cv == NULL) {
        return;
    }
    
    mutex_lock(cv->mutex);
    listnode_t *node = linklist_remove_head(cv->queue);
    mutex_unlock(cv->mutex);
    
    if (node == NULL) {
        return;
    }
    
    int tid = (int)node->data;
    if (make_runnable(tid) == 0) {
        yield(tid);
    }
    
    free(node);    
}

/** @brief Wakes up all threads waiting on a condition variable.
 *
 *  @param list Condition variable.
 *  @return Void.
 */
void cond_broadcast(cond_t *cv) {
    if (cv == NULL) {
        return;
    }
    
    mutex_lock(cv->mutex);
    listnode_t *node = linklist_remove_all(cv->queue);
    mutex_unlock(cv->mutex);
    
    while (node != NULL) {
        int tid = (int)node->data;
        if (make_runnable(tid) == 0) {
            yield(tid);
        }
        
        listnode_t *oldnode = node;
        node = node->next;
        
        free(oldnode);       
    }
}
