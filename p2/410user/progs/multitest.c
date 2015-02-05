/**
 * @file multitest.c
 *
 * @brief A crazy test of the thread implementation.
 *
 * This program keeps one copy of a bunch of thread-based
 * programs running at once.
 *
 * @author Alejandro Lince (alince)
 * @author Spencer Whitman (swhitman)
 *
 * @bug None known
 */

#include <syscall.h>
#include <simics.h>
#include <mutex.h>
#include <stdio.h>
#include <thread.h>
#include <sem.h>

#define TESTS 3

int main()
{
  int i;
  int count = 0;
  char * tests[] = {"agility_drill", "cvar_test", "cyclone", 
		     "join_specific_test", "thr_exit_join"};

  char * ad[] = {"agility_drill",(char *)0};
  char * ct[] = {"cvar_test",(char *)0};
  char * c[] = {"cyclone",(char *)0};
  char * jst[] = {"join_specific_test",(char *)0};
  char * tej[] = {"thr_exit_join",(char *)0};

  char ** argsvec[] = { ad,ct,c,jst,tej};
  
   while(1) {
    lprintf("-----------------Iteration: %d-----------------------\n",count++);
    for(i=0;i<TESTS;i++) {
      sleep(5);
      misbehave(-1);
      int tid = fork();
      
      if(tid == 0) {
	printf("execing %s\n",tests[i]);
	exec(tests[i],argsvec[i]);
	printf("test %s could not exec\n",tests[i]);
	return 0;
      }
      
    }
  }

}
