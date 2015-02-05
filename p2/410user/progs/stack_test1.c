/** @file 410user/progs/stack_test1.c
 *  @author efaust
 *  @brief Tests whether auto-stack pages are zeroed.
 *  @public yes
 *  @for p3
 *  @covers autostack
 *  @status done
 *
 *  Note: If we ever go back to the kernel-level autostack, we should revert
 *  this to the old one.
 */

#include <stdio.h>
#include <stdlib.h>
#include <syscall.h>
#include "410_tests.h"
#include <report.h>

DEF_TEST_NAME("stack_test1:");

#define SIZE (65536)

/* Can't you see that I am not afraid */
void touch_me_babe(char *arr, size_t len)
{
	size_t i;
	for(i = 0; i < len; i++) {
		arr[i] = i;
	}
}

void do_test()
{
	char ca[SIZE];
	touch_me_babe(ca, sizeof(ca));
}

int main()
{

  report_start(START_CMPLT);

  do_test();
  report_misc("stack allocation successful");
  report_end(END_SUCCESS);
  exit(42);
}
