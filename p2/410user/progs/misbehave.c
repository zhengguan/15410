/** @file misbehave.c
 *
 * @brief Test program for 15-412 project 3 Spring 2003
 * @author Updated 4/2/04 by Mark T. Tomczak (mtomczak)
 *
 * Switches kernel misbehave mode.
 */

/* Includes */
#include <syscall.h>
#include <test.h>          /* assuredly_misbehave() */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Main */
int main( int argc, char *argv[] )
{
  int misbehave_val;
        if( argc == 1 ) {
                printf("usage: misbehave <mode>");
                exit( -1 );
        }
  misbehave_val = atoi(argv[1]);

  assuredly_misbehave(misbehave_val);

	return 0;
}
