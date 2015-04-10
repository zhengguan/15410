/** @file readonly_test.c
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <stdlib.h>
#include <stdio.h>
#include <simics.h>

volatile const int a = 4;

void fucked_up()
{
    return;
}

int main(int argc, char* argv[])
{
    lprintf("about to break");
    *(int*)fucked_up = 21;
    return 0;
}