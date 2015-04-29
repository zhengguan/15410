/** @file disk.c
 *  @brief This file implements the disk I/O system calls.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <stdlib.h>
#include <syscall.h>
#include <exec2obj.h>
#include <ide.h>
#include <fs.h>
#include <kern_common.h>
#include <assert.h>

#define SUPERBLOCK_ADDR 0

#define SECTOR_MASK (~(IDE_SECTOR_SIZE - 1))
#define PREV_SECTOR(OFFSET) ((unsigned)(OFFSET) & SECTOR_MASK)
#define NEXT_SECTOR(OFFSET) (PREV_SECTOR((OFFSET) + IDE_SECTOR_SIZE))

typedef struct superblock {
    int constant;
    int file_node;
    int free_node;
    char padding[IDE_SECTOR_SIZE - 12];
} superblock_t;

typedef struct free_node {
    int next;
    int len;
    char padding[IDE_SECTOR_SIZE - 8];
} free_node_t;

typedef struct file_node {
    int next;
    char filename[MAX_EXECNAME_LEN];
    uint32_t size;
    int writeable;
    int data_node;
    char padding[IDE_SECTOR_SIZE - 16 -
                 MAX_EXECNAME_LEN*sizeof(char)];
} file_node_t;

typedef struct data_node {
    int next;
    int len;
    int start;
    char padding[IDE_SECTOR_SIZE - 12];
} data_node_t;

static int read_superblock(superblock_t *superblock) {
    int rv = dma_read(SUPERBLOCK_ADDR, (void *)superblock, 1);
    if (!rv)
        assert(superblock->constant == FS_MAGIC_CONSTANT);
    return rv;
}

static int read_file_node(unsigned long addr, file_node_t *file_node) {
    return dma_read(addr, (void *)file_node, 1);
}

static int read_data_node(unsigned long addr, data_node_t *data_node) {
    return dma_read(addr, (void *)data_node, 1);
}

static int write_superblock(superblock_t *superblock) {
    superblock->constant = FS_MAGIC_CONSTANT;
    return dma_write(SUPERBLOCK_ADDR, (void *)superblock, 1);
}

static int write_file_node(unsigned long addr, file_node_t *file_node) {
    return dma_write(addr, (void *)file_node, 1);
}

static int write_free_node(unsigned long addr, free_node_t *free_node) {
    return dma_write(addr, (void *)free_node, 1);
}

static int get_file_node(char *filename, file_node_t *file_node) {
    superblock_t *superblock = malloc(sizeof(superblock_t));
    if (superblock == NULL)
        return -1;
    if (read_superblock(superblock) < 0) {
        free(superblock);
        return -2;
    }

    int addr = superblock->file_node;
    free(superblock);
    while (addr != 0) {
        if (read_file_node(addr, file_node) < 0)
            return -3;
        if (!strncmp(file_node->filename, filename, MAX_EXECNAME_LEN))
            return 0;
        addr = file_node->next;
    }

    return -4;
}

static int remove_file_node(char *filename, file_node_t *file_node) {
    superblock_t *superblock = malloc(sizeof(superblock_t));
    if (superblock == NULL)
        return -1;
    if (read_superblock(superblock) < 0) {
        free(superblock);
        return -2;
    }

    int rv = -4;

    int prev_addr = 0;
    int addr = superblock->file_node;
    while (addr != 0) {
        if (read_file_node(addr, file_node) < 0) {
            addr = -5;
            break;
        }
        if (!strncmp(file_node->filename, filename, MAX_EXECNAME_LEN)) {
            rv = 0;
            int next = file_node->next;
            if (prev_addr == 0) {
                superblock->file_node = next;
                write_superblock(superblock);
            } else {
                if (read_file_node(prev_addr, file_node) < 0)
                    addr = -6;
                file_node->next = next;
                if (write_file_node(prev_addr, file_node) < 0)
                    addr = -7;
            }
            break;
        }

        prev_addr = addr;
        addr = file_node->next;
    }

    free(superblock);

    return addr;
}

static int add_free_node(unsigned long addr, int len, free_node_t *free_node) {
    superblock_t *superblock = malloc(sizeof(superblock_t));
    if (superblock == NULL)
        return -1;
    if (read_superblock(superblock) < 0) {
        free(superblock);
        return -2;
    }

    free_node->next = superblock->free_node;
    free_node->len = len;
    superblock->free_node = addr;

    if (write_superblock(superblock) < 0) {
        free(superblock);
        return -3;
    }
    free(superblock);
    if (write_free_node(addr, free_node) < 0)
        return -4;

    return 0;
}

static int ls(char *buf, int count) {
    superblock_t *superblock = malloc(sizeof(superblock_t));
    if (superblock == NULL)
        return -1;
    if (read_superblock(superblock) < 0) {
        free(superblock);
        return -2;
    }
    int addr = superblock->file_node;
    free(superblock);

    file_node_t *file_node = malloc(sizeof(superblock_t));
    if (file_node == NULL)
        return -3;

    int read_len = 0;

    while (addr != 0) {
        if (read_file_node(addr, file_node) < 0) {
            read_len = -4;
            break;
        }
        int len = MIN(strlen(file_node->filename) + 1, count - read_len);
        memcpy(buf + read_len, file_node->filename, len);
        read_len += len;
        if (read_len == count)
            break;
        addr = file_node->next;
    }

    free(file_node);

    return read_len;
}

int readfile(char *filename, char *buf, int count, int offset)
{
    if (!strcmp(filename, "."))
        return ls(buf, count);

    char *kernel_buf = malloc(count * sizeof(char));
    if (kernel_buf == NULL)
        return -1;

    file_node_t *file_node = malloc(sizeof(file_node_t));
    if (file_node == NULL) {
        free(kernel_buf);
        return -2;
    }
    if (get_file_node(filename, file_node) < 0) {
        free(kernel_buf);
        return -3;
    }

    data_node_t *data_node = malloc(sizeof(data_node_t));
    if (data_node == NULL) {
        free(kernel_buf);
        free(file_node);
        return -4;
    }

    int read_len = 0;

    int addr = file_node->data_node;
    free(file_node);
    while (addr != 0) {
        if (read_data_node(addr, data_node) < 0) {
            read_len = -5;
            break;
        }

        int sector = data_node->start;
        int extent_bytes = data_node->len * IDE_SECTOR_SIZE;

        // Skip entire extent
        if (offset >= extent_bytes) {
            offset -= extent_bytes;
            addr = data_node->next;
            continue;
        }

        // Read first sector
        if (offset > 0) {
            sector += offset / IDE_SECTOR_SIZE;
            offset = offset - PREV_SECTOR(offset);
        }
        if (offset > 0) {
            char tmp_buf[IDE_SECTOR_SIZE];
            if (dma_read(sector, tmp_buf, 1) < 0) {
                read_len = -6;
                break;
            }
            int len = MIN(count, NEXT_SECTOR(offset) - offset);
            memcpy(kernel_buf, tmp_buf + offset, len);
            offset = 0;
            read_len += len;
            sector++;
        }

        // Read multiple sectors
        if (count - read_len >= IDE_SECTOR_SIZE &&
            sector < data_node->start + data_node->len) {
            int sector_len = MIN((count - read_len) / IDE_SECTOR_SIZE, data_node->start + data_node->len - sector);
            if (dma_read(sector, kernel_buf + read_len, sector_len) < 0) {
                read_len = -7;
                break;
            }
            read_len += sector_len * IDE_SECTOR_SIZE;
            sector += sector_len;
        }

        // Read last sector
        if (count - read_len > 0 &&
            sector < data_node->start + data_node->len) {
            char tmp_buf[IDE_SECTOR_SIZE];
            if (dma_read(sector, tmp_buf, 1) < 0) {
                read_len = -8;
                break;
            }
            int len = count - read_len;
            memcpy(kernel_buf + read_len, tmp_buf, len);
            read_len += len;
            sector++;
        }

        if (read_len == count)
            break;

        addr = data_node->next;
    }

    if (read_len > 0)
        memcpy(buf, kernel_buf, read_len);

    free(kernel_buf);
    free(data_node);

    return read_len;
}

int sizefile(char *filename)
{
    file_node_t *file_node = malloc(sizeof(file_node_t));
    if (file_node == NULL)
        return -2;
    if (get_file_node(filename, file_node) < 0) {
        free(file_node);
        return -1;
    }

    free(file_node);

    return file_node->size;
}

int writefile(char *filename, char *buf, int count, int offset, int create)
{
    return -1;
}

int deletefile(char *filename)
{
    file_node_t *file_node = malloc(sizeof(file_node_t));
    if (file_node == NULL)
        return -1;
    int file_addr = remove_file_node(filename, file_node);
    if (file_addr < 0) {
        free(file_node);
        return -2;
    }
    int data_addr = file_node->data_node;
    free(file_node);

    data_node_t *data_node = malloc(sizeof(data_node_t));
    if (data_node == NULL)
        return -4;

    free_node_t *free_node = malloc(sizeof(data_node_t));
    if (free_node == NULL) {
        free(data_node);
        return -5;
    }

    add_free_node(file_addr, 1, free_node);

    int rv = 0;

    while (data_addr != 0) {
        if (read_data_node(data_addr, data_node) < 0) {
            rv = -7;
            break;
        }
        add_free_node(data_addr, 1, free_node);
        add_free_node(data_node->start, data_node->len, free_node);
        data_addr = data_node->next;
    }

    free(data_node);
    free(free_node);

    return rv;
}