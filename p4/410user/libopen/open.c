/* @file open.h
 * @brief Implementation for POSIX-style open()/close()/read()/write()/lseek()
 * file IO calls.
 * @author Chris Williamson (cdw1)
 */

#include <stdlib.h>
#include <string.h>
#include <syscall.h>
#include <open.h>

typedef struct {
    char fname[256];
    int seek;
    int fsize;
    int mode; // 0 == closed
} filedes_t;

static filedes_t fd_tab[MAX_OPEN_FD];
static int num_open_fds = 0;

int open(const char *filename, int flags)
{
    int rc, fsize, fd, namelen;

    if (num_open_fds == MAX_OPEN_FD) {
        return -1;
    }

    if (filename == NULL) {
        return -1;
    }
    namelen = strlen(filename);
    if (namelen == 0 || namelen > MAX_FNAME) {
        return -1;
    }

    /* Validate flags: O_CREAT can be set independently, but O_RDONLY and
       O_RDWR must be mutually exclusive */
    if (flags & O_CREAT) {
        int f = flags & (~O_CREAT);
        if ((f != O_RDONLY) && (f != O_RDWR)) {
            return -1;
        }
    } else {
        if ((flags != O_RDONLY) && (flags != O_RDWR)) {
            return -1;
        }
    }

    /* Get the file's size and check read/write bits */
    fsize = sizefile((char *)filename);
    if (fsize < 0) {
        if (flags & O_CREAT) {
            rc = writefile((char *)filename, NULL, 0, 0, 1);
            if (rc < 0)
                return rc;
        } else {
            return -1;
        }
    }
#ifdef notdef
    /* Check permissions */
    if (stat(STAT_RDWR) == 0 && (flags & O_RDWR)) {
        return -1;
    }
#endif

    /* Find an open table slot and claim it */
    for (fd = 0; fd < MAX_OPEN_FD; fd++) {
        if (fd_tab[fd].mode == 0)
            break;
    }
    num_open_fds++;
    fd_tab[fd].mode = flags;
    fd_tab[fd].fsize = fsize;
    fd_tab[fd].seek = 0;
    strncpy(fd_tab[fd].fname, filename, MAX_FNAME);

    return fd;
}

int close(int fd)
{
    if (fd < 0 || fd >= MAX_OPEN_FD ||
        fd_tab[fd].mode == 0) {
        return -1;
    }

    fd_tab[fd].fname[0] = '\0';
    fd_tab[fd].seek = 0;
    fd_tab[fd].fsize = 0;
    fd_tab[fd].mode = 0;

    return 0;
}

int read(int fd, void *buf, size_t count)
{
    size_t to_read;
    int rc;

    if (fd < 0 || fd >= MAX_OPEN_FD || buf == NULL || count <= 0 ||
        fd_tab[fd].mode == 0) {
        return -1;
    }

    if (fd_tab[fd].seek >= fd_tab[fd].fsize) {
        return 0;
    }

    if (count < (fd_tab[fd].fsize - fd_tab[fd].seek)) {
        to_read = count;
    } else {
        to_read = fd_tab[fd].fsize - fd_tab[fd].seek;
    }

    if ((rc = readfile(fd_tab[fd].fname, buf, to_read, fd_tab[fd].seek)) < 0)
        return rc;

    fd_tab[fd].seek += rc;

    return rc;
}

int write(int fd, void *buf, size_t count)
{
    int rc;

    if (fd < 0 || fd >= MAX_OPEN_FD || buf == NULL || count <= 0 ||
        !(fd_tab[fd].mode & O_RDWR)) {
        return -1;
    }

    if ((rc = writefile(fd_tab[fd].fname, buf, count, fd_tab[fd].seek, 0)) < 0)
        return rc;

    fd_tab[fd].seek += rc;
    fd_tab[fd].fsize = sizefile((char *) fd_tab[fd].fname);

    return rc;
}

int lseek(int fd, int offset, int whence)
{
    if (fd < 0 || fd >= MAX_OPEN_FD || fd_tab[fd].mode == 0) {
        return -1;
    }

    if (whence == SEEK_SET) {
        fd_tab[fd].seek = offset;
    } else if (whence == SEEK_CUR) {
        fd_tab[fd].seek += offset;
    } else if (whence == SEEK_END) {
        fd_tab[fd].seek = fd_tab[fd].fsize + offset;
    } else {
        return -1;
    }

    return (fd_tab[fd].seek);
}
