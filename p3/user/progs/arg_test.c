/** @file readfile.c
 *  @brief This file implements the readfile system call.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char* argv[])
{
    int i;
    printf("argc: %d\n", argc);
    for (i = 0; i < argc; i++) {
        printf("%s\n", argv[i]);
    }
    return 0;
}