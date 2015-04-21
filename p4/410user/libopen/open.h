/* @file open.h
 * @brief Defines an interface around readfile() and writefile() which matches
 * the POSIX-style open()/close()/read()/write()/lseek() file IO calls.
 * Intended to be called in a single-threaded context or with external locking.
 * @author Chris Williamson (cdw1)
 */

#ifndef _OPEN_H_
#define _OPEN_H_

#include <types.h>

#define MAX_FNAME   255

#define O_CREAT     0x01
#define O_RDONLY    0x02
#define O_RDWR      0x04

#define SEEK_SET    1
#define SEEK_CUR    2
#define SEEK_END    3

#define MAX_OPEN_FD 32

int open(const char *filename, int flags);
int close(int fd);
int read(int fd, void *buf, size_t count);
int write(int fd, void *buf, size_t count);
int lseek(int fd, int offset, int whence);

#endif /* _OPEN_H_ */

