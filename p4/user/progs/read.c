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
	char buf[BUFSIZE];
	int i;
	for (i = 1; i < argc; i++) {
		char *file = argv[i];
		int offset = 0;
		while (offset < READ_LEN) {
			int amt_read = readfile(file, buf, BUFSIZE, offset);
			if (amt_read < 0)
				return -1 * i;
			print(amt_read, buf);
			offset += amt_read;
		}
	}
	return 0;
}
