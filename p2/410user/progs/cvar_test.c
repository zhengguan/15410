/** @file cvar_test.c
 *
 *  @brief Test program for condition variables
 *
 *  write full description here
 *
 *  Tests: WHAT THIS TESTS
 *
 *  @author Rahul Iyer (rni)
 *
 *  @bug None known
 *
 *  Something condition-variable-intensive.
 *
 *  One thing I'd like to see
 *  is a thread which goes to sleep on one condition variable (gl: C) and hollers
 *  if it is awakened too soon (before a flag is set).  While it's asleep,
 *  "a whole bunch" of other threads sleep and wake up on some other
 *  condition variable(s).
 *
 *  Maybe some freaky pattern, like whenever a
 *  thread wakes up on cvar A it does a signal(B) and waits on B and
 *  whenever a thread wakes up on cvar B it does a broadcast(A) and
 *  waits on A, though you'd probably need an external thread called
 *  "instigator" to keep poking both of them every 5 sleep ticks or so.
 *
 *  Everybody would of course be banging on the same "world mutex".
 *
 *  After a while the instigator would clean things up by acquiring
 *  the global mutex, setting a "done" flag, broadcasting on all
 *  three condition variables, and exiting.  Everybody else would
 *  then wake up, check the flag, and exit.
 *
 *  Maybe the program name
 *  would be "unconditional" (or "instigator"), and maybe it would
 *  take command line flags for how many "worker" threads there would
 *  be and how many sleep()/poke cycles the instigator would go
 *  through before declaring the game over, with both defaulting
 *  to something reasonable.
 */

#include <thread.h>
#include <stdio.h>
#include <stddef.h>
#include <syscall.h>
#include <simics.h>
#include <stdlib.h>
#include <mutex.h>
#include <cond.h>
#include <thrgrp.h>

#define STACK_SIZE 4096
#define NR_MANIACS 10

mutex_t big_global_mutex;
int done = 0;
int cleanup=0;
cond_t a; /* don't try these global variable names at home, kids */
cond_t b;
cond_t c;

void * sleeper_thread(void * arg)
{
        mutex_lock(&big_global_mutex);

        cond_wait(&c, &big_global_mutex);

        if (!done) {
                lprintf("It's too early to be up :(");
                exit(1);
        }

        lprintf("Good Morning!");
        thr_exit(NULL);
	return NULL;
}

void *maniac(void *arg)
{
	int tid = thr_getid();

	mutex_lock(&big_global_mutex);

	lprintf("%d sleeping! on A", tid);
	cond_wait(&a, &big_global_mutex);

	lprintf("%d woken up on A!", tid);

	cond_signal(&b);
	
	lprintf("%d sleeping! on B", tid);
	cond_wait(&b, &big_global_mutex);
	
	lprintf("%d woken up on B!", tid);
	
	cond_broadcast(&a);
	
	lprintf("%d sleeping! on A", tid);
	cond_wait(&a, &big_global_mutex);
	
	lprintf("%d woken up on A!", tid);
	
	mutex_unlock(&big_global_mutex);
  
  /* this has to be a return now, need to exit on grp*/
	return NULL;
}

void *instigator(void *arg)
{
	while(!cleanup) {
		sleep(5);
		lprintf("instigator broadcasting on A");
		cond_signal(&a);
		lprintf("instigator broadcasting on B");
		cond_signal(&b);
	}
	
        thr_exit(NULL);
	return NULL;
}


int main( int argc, char *argv[] ) 
{
	int sleeper_tid;
	int instigator_tid;
	int i;
  thrgrp_group_t maniac_grp;

	thr_init(STACK_SIZE);
	
	mutex_init(&big_global_mutex);
	cond_init(&a);
	cond_init(&b);
	cond_init(&c);
  thrgrp_init_group(&maniac_grp);
	
	
	if ((sleeper_tid=thr_create(sleeper_thread, NULL)) < 0) {
		lprintf("Error in thread Creation!");
		exit(1);
	}

	thr_yield(sleeper_tid);

	for (i=0;i<NR_MANIACS;++i) {
		if (thrgrp_create(&maniac_grp, maniac, NULL) < 0) {
			lprintf("Error in thread Creation!");
			exit(1);
		}

		thr_yield(-1);
	}

	if ((instigator_tid=thr_create(instigator, NULL)) < 0) {
                lprintf("Error in thread Creation!");
                exit(1);
        }
	thr_yield(instigator_tid);

	
	for (i=0;i<NR_MANIACS; ++i)
		thrgrp_join(&maniac_grp, NULL);
		
	done=1;
	cond_signal(&c);

	thr_join(sleeper_tid, NULL);
	cleanup=1;
	thr_join(instigator_tid, NULL);

        thr_exit(NULL);
	return 0;
	
}
