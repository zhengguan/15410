/** @file excellent.c
 *
 * @brief A multi-threaded application which sometimes experiences
 * a thread crash.
 *
 * @note  In some sense, this program is a "riddle", i.e., we hope
 * that providing it to you will help you think about what should
 * happen in certain circumstances.  So please don't send us mail
 * asking "What should happen when we run 'excellent'?"...
 *
 * @author de0u (S'12)
 **/

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>

#include <syscall.h>
#include <thread.h>
#include <test.h>

void *worker(void* input)
{
	if ((gettid() % 3) == 0) {
		printf("***********************************************************************\n");
		printf("* I believe our adventure through time has taken a most serious turn. *\n");
		printf("***********************************************************************\n");
		illegal();
	}
	return input;
}

int
main(int argc, char *argv[])
{
	int tid, error, answer=69;
	void *tstatus;

	if (thr_init(65536) < 0)
		panic("BOGUS: thr_init() failed!");

	if ((tid = thr_create(worker, (void *)answer)) < 0)
		panic("HEINOUS: thr_create() failed!");

	if ((error = thr_join(tid, &tstatus)) != 0)
		panic("MOST NON-TRIUMPHANT: thr_join() failed!");

	if ((int)tstatus != answer)
		panic("BOGUS!!");

	printf("*******************************\n");
	printf("* Be excellent to each other. *\n");
	printf("*******************************\n");
	thr_exit(0);
	panic("BOGUS!!");
	return (99);
}
