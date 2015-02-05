/** @file 410user/progs/switzerland.c
 *  @author rpearl
 *  @brief Tests whether the auto-stack survives thr_init()
 *  @public yes
 *  @for p2
 *  @covers autostack
 *  @status done
 */

#include <stdio.h>
#include <thread.h>
#include <stdlib.h>
#include <syscall.h>
#include "410_tests.h"
#include <report.h>

DEF_TEST_NAME("switzerland:");

#define SIZE         (65536)

#define NEUTRAL_ZONE (15410)
#define CHEESE       (8192)

void invade(char *arr, size_t len)
{
	size_t i;
	for(i = 0; i < len; i++) {
		arr[i] = i;
	}
}

void do_test()
{
	char ca[SIZE];
	invade(ca + NEUTRAL_ZONE, CHEESE);

	if (thr_init(2 * SIZE) < 0) {
		report_misc("thr_init() failed?");
		report_end(END_FAIL);
		exit(-1);
	}

	invade(ca, sizeof (ca));
}

int main()
{

  report_start(START_CMPLT);

  do_test();
  report_misc("stack allocation successful");
  report_end(END_SUCCESS);
  thr_exit((void *)42);
  return(42);
}
