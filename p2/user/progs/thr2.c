#include <thread.h>
#include <mutex.h>
#include <simics.h>
#include <stdlib.h>
#include <stdio.h>

static mutex_t mutex;

#define NUM_THREADS 10

void *thread_two(void *a) {
	mutex_lock(&mutex);
	printf("hello from %d: %d\n", (int)a, thr_getid());
	mutex_unlock(&mutex);

	return NULL;
}

int main()
{

	mutex_init(&mutex);
	thr_init(100);

	int tids[NUM_THREADS];

	int i;
	for (i = 0; i < NUM_THREADS; i++) {
		tids[i] = thr_create(thread_two, (void*)i);
	}

	mutex_lock(&mutex);
	printf("hello from main: %d\n", thr_getid());
	mutex_unlock(&mutex);

	for (; i > 0; i--)
		thr_join(tids[i], NULL);

	return 0;

}