/** @file mutex.c
 *  @brief This file implements the interface for mutexes.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <sem.h>
#include <stdlib.h>
#include <mutex.h>
#include <cond.h>

/**
 * @brief Initializes a semaphore
 * @param sem The semaphore
 * @param count The initial count of the semaphore
 *
 * @return 0 if completes successfully and an error code less than 0 otherwise
 */
int sem_init(sem_t *sem, int count)
{
    if (sem == NULL || count < 1)
        return -1;

    sem->count = count;

    if (mutex_init(&sem->count_mutex) < 0)
        return -2;
    if (cond_init(&sem->cond) < 0)
        return -3;

    sem->valid = 1;

    return 0;
}

/**
 * @brief Waits until a slot becomes open in the semaphore
 * @param sem The semaphore
 */
void sem_wait(sem_t *sem)
{
    if (sem == NULL || !sem->valid)
        return;

    mutex_lock(&sem->count_mutex);
    while (sem->count == 0)
        cond_wait(&sem->cond, &sem->count_mutex);
    sem->count--;
    mutex_unlock(&sem->count_mutex);
}

/**
 * @brief Signals that a slot is open in the semaphore
 * and awakes a waiting thread
 * @param sem The semaphore
 */
void sem_signal(sem_t *sem)
{
    if (sem == NULL)
        return;

    mutex_lock(&sem->count_mutex);
    sem->count++;
    mutex_unlock(&sem->count_mutex);
    cond_signal(&sem->cond);
}

/**
 * @brief Destroys a semaphore
 * @param sem The semaphore
 */
void sem_destroy(sem_t *sem)
{
    if (sem == NULL)
        return;

    cond_destroy(&sem->cond);
    mutex_destroy(&sem->count_mutex);
    sem->valid = 0;
}

