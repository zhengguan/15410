#include <thread.h>
#include <sem.h>
#include <syscall.h>
#include <simics.h>
#include <stdlib.h>
#include <stdio.h>

#define SEM_COUNT 3
#define NUM_THREADS 8

static mutex_t mutex;
static sem_t sem;
int count;

void *child_thr(void *arg) {
    sem_wait(&sem);
    mutex_lock(&mutex);
    lprintf("Thread %2d entered semaphore.   Count = %2d", gettid(), ++count);
    mutex_unlock(&mutex);
    sleep((int)arg);
    mutex_lock(&mutex);
    lprintf("Thread %2d exited semaphore.   Count = %2d", gettid(), --count);
    mutex_unlock(&mutex);
    sem_signal(&sem);
    
	return (void*)42;
}

int main() {
    int ret;
    
    mutex_init(&mutex);
    ret = sem_init(&sem, SEM_COUNT);
    if (ret < 0) {
        return ret;
    }
    count = 0;
	thr_init(4096);

    int tids[NUM_THREADS];

    int i;
    for (i=0; i < NUM_THREADS; i++) {
        tids[i] = thr_create(child_thr, (void *)(i % 4));
    }

    for (i=0; i < NUM_THREADS; i++) {
        thr_join(tids[i], (void **)&ret);
    }

	return 0;
}
