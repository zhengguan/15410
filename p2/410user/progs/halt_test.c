/** @file 410user/progs/halt_test.c
 *  @author mpa
 *  @brief Tests the basic functionality of halt()
 *  @public yes
 *  @for p2 p3
 *  @covers halt
 *  @status done
 */

#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <simics.h>
#include "410_tests.h"

DEF_TEST_NAME("halt_test:");

int main()
{
  REPORT_START_ABORT;

  halt();

  REPORT_END_FAIL;
  exit(-1);
}
