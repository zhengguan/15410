/** @file 410user/progs/wild_test1.c
 *  @author zra
 *  @brief Tests thread death by unnatural causes.
 *  @public yes
 *  @for p2 p3
 *  @covers faults
 *  @status done
 */

/* Includes */
#include <syscall.h>  /* was for exit */
#include <stdlib.h>   /* for exit */
#include <stdio.h>
#include <simics.h>   /* for lprintf */

#include "410_tests.h"
DEF_TEST_NAME("wild_test1:");

/* Main */
int main(int argc, char *argv[])
{
	int *wild = (int *)0x31337000;

	REPORT_START_ABORT;
	

	*wild = 0x2badd00d;

  REPORT_END_FAIL;

	exit( -1 );
}
