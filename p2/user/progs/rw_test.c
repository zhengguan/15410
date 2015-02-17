#include <thread.h>
#include <rwlock.h>
#include <syscall.h>
#include <simics.h>
#include <stdlib.h>
#include <stdio.h>

#define NUM_READERS 5
#define NUM_WRITERS 2

static rwlock_t rwlock;

void *reader_thr(void *arg) {
    rwlock_lock(&rwlock, RWLOCK_READ);
    lprintf("Reader %d aquired lock", gettid());
    sleep(2);
    rwlock_unlock(&rwlock);
    lprintf("Reader %d released lock", gettid());
    
	return (void*)42;
}

void *writer_thr(void *arg) {
    rwlock_lock(&rwlock, RWLOCK_WRITE);
    lprintf("Writer %d aquired lock", gettid());
    // rwlock_downgrade(&rwlock);
    // lprintf("Writer %d downgraded", gettid());
    rwlock_unlock(&rwlock);
    lprintf("Writer %d released lock", gettid());
    
	return (void*)42;
}

int main() {    
	int ret; 
	
	ret = rwlock_init(&rwlock);
	if (ret < 0) {
	    return ret;
	}
	thr_init(4096);

    int tids[NUM_READERS + NUM_WRITERS];

    int i;
    for (i=0; i < NUM_READERS + NUM_WRITERS; i++) {
        if (i < NUM_READERS) {
            tids[i] = thr_create(reader_thr, NULL);
        } else {
            tids[i] = thr_create(writer_thr, NULL);
        }
    }
    
    for (i=0; i < NUM_READERS + NUM_WRITERS; i++) {
        thr_join(tids[i], (void **)&ret);
    }

	return 0;
}
