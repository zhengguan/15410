#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <simics.h>

int main(int argc, char **argv)
{
	if (argc > 1)
        return deletefile(argv[1]);
    return -1;
}
