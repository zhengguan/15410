// Simple program to print out files.
// Michael J. Sullivan, Spring 2013

#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <simics.h>

#define BUFSIZE 100
#define READ_LEN 3000

int main(int argc, char **argv)
{
	int i;
	for (i = 1; i < argc; i++) {
		char *file = argv[i];
		int size = sizefile(file);
		if (size <= 0)
			return -1 * i;
		char sizestr[10];
		sprintf(sizestr, "%d", size);
		print(10, sizestr);
	}
	return 0;
}
