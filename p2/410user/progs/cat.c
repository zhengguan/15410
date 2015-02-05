// Simple program to print out files.
// Michael J. Sullivan, Spring 2013

#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>

#define BUFSIZE 87 // "four score and seven"

int main(int argc, char **argv)
{
	char buf[BUFSIZE];
	int i;
	for (i = 1; i < argc; i++) {
		char *file = argv[i];
		int offset = 0;
		int amt_read;
		while ((amt_read = readfile(file, buf, BUFSIZE, offset)) > 0) {
			print(amt_read, buf);
			offset += amt_read;
		}
		if (amt_read < 0) {
			printf("error reading from file %s at offset %d\n",
			       file, offset);
			return -1;
		}
	}

	return 0;
}
