#include <thread.h>
#include <mutex.h>
#include <simics.h>
#include <stdlib.h>
#include <stdio.h>

static mutex_t mutex;

#define NUM_THREADS 10

void *die(void *a) {
	return *(void**)0;
}

int main()
{

	mutex_init(&mutex);
	thr_init(100);

	thr_join(thr_create(die, NULL), NULL);


	return 0;
}