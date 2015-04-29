#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <simics.h>

int main(int argc, char **argv)
{
	return writefile("hello_world.txt", NULL, 42, 24, 0);
}
