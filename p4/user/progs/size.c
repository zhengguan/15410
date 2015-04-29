#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <simics.h>

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
