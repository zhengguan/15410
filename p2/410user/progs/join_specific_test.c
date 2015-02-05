/** @file join_specific_test.c
 *
 *  @brief Joins on a specific thread
 *
 *  This test spawns several threads and joins on some of them specifically.
 *  Others are joined on in a more general fashion.
 *
 *  Proper behavior: when the function executes properly, each of 14 threads
 *  gets one of the letters in ``Hello, World!'' They then print their letter
 *  and return.
 *
 *  When functioning properly, the program will output some permutation of
 *  the characters in ``Hello, World!'' (the exact permutation depends on
 *  the order the threads return... and therefore the behavior of the
 *  scheduler and thread library).
 *
 *  Tests: syscalls, join on specific threads, malloc (thread-safety
 *         unnecessary)
 * 
 *  @author Mark T. Tomczak (mtomczak)
 *  @author Andy Herrman (aherrman)
 *
 *  @bug None known
 **/

#include <stdlib.h>
#include <stddef.h>
#include <syscall.h>
#include <simics.h>
#include <thread.h>
#include <stdio.h>
#include <string.h>

#define stackSize 4096

#define SUCCESS 0



typedef struct myArgs{
	char cookie; /** each of my threads.... gets a cookie :) **/
} myArgs;

void *baseFunc (void* args)
{
	myArgs *argsin = (myArgs *)args;

	yield(-1);
	print (1, &(argsin->cookie));

	return args;
}

int main(int argc, char **argv)
{
	int error;
	int tids[30];
	int i;
	myArgs *curArg;
	char *myOutput="Hello, world!\n";
	
	thr_init(stackSize);

	for (i=0; i<strlen(myOutput); i++)
		{
			curArg=malloc(sizeof(myArgs));
			
			curArg->cookie=myOutput[i];
			
			tids[i]=thr_create(baseFunc, (void *)curArg);
		}

	for(i=strlen(myOutput)-1; i>=0; i--)
		{
			if((error = thr_join(tids[i], (void **)&curArg)) !=SUCCESS)
				{
					lprintf("Thr_join error %d\n",error);
				}

			if(curArg != NULL)
				{
					free(curArg);
				}
		}

	thr_exit(0);
	return 0;
}
