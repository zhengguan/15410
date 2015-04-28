// Simple program to print out files.
// Michael J. Sullivan, Spring 2013

#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <simics.h>

#define BUFSIZE 1000
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

int main(int argc, char **argv)
{
	char *file = argc > 1 ? argv[1] : "Advent.txt";
	int count = argc > 2 ? atoi(argv[2]) : 512;
	int offset = argc > 3 ? atoi(argv[3]) : 0;
	char buf[BUFSIZE];
	int read_len = offset + count;
	while (offset < read_len) {
		int len = MIN(read_len - offset, BUFSIZE);
		int amt_read = readfile(file, buf, len, offset);
		if (amt_read < 0)
			return -1;
		print(amt_read, buf);
		offset += amt_read;
	}
	return 0;
}
