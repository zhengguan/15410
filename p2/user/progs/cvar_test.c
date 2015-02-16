#include <thread.h>
#include <mutex.h>
#include <cond.c>
#include <syscall.h>
#include <simics.h>
#include <stdlib.h>
#include <stdio.h>

static mutex_t mutex;
static cond_t cvar;

void *signal_thr(void *arg) {
    mutex_lock(&mutex);
    printf("Signal thread aquired mutex\n");
    printf("Signal thread sending signal\n");
	cond_broadcast(&cvar);
	mutex_unlock(&mutex);
    printf("Signal thread released mutex\n");
    
	return (void*)42;
}

void *wait_thr(void *arg) {
    mutex_lock(&mutex);
    printf("Wait thread aquired mutex\n");
    printf("Wait thread beginning to wait\n");
	cond_wait(&cvar, &mutex);
    printf("Wait thread done waiting\n");
	mutex_unlock(&mutex);
    printf("Wait thread released mutex\n");
    
	return (void*)42;
}

int main()
{
    int ret;
    
	ret = mutex_init(&mutex);
	if (ret < 0) {
	    lprintf("mutex_init failed\n");
	    return ret;
	}
	ret = cond_init(&cvar);
	if (ret < 0) {
	    lprintf("cond_init failed\n");
	    return ret;
	}
	thr_init(4096);


    mutex_lock(&mutex);
	int wait_tid = thr_create(wait_thr, NULL);
	lprintf("Wait_tid: %d", wait_tid);
	int signal_tid = thr_create(signal_thr, NULL);
	lprintf("Signal_tid: %d", signal_tid);
    printf("Main thread aquired mutex\n");
    printf("Main thread beginning to wait\n");
	cond_wait(&cvar, &mutex);
    printf("Main thread done waiting\n");
	mutex_unlock(&mutex);
    printf("Main thread released mutex\n");
    
	thr_join(wait_tid, (void**)&ret);
	thr_join(signal_tid, (void**)&ret);
	lprintf("Threads returned %d", ret);

	return 0;
}
