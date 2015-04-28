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

// static int read_free_node(unsigned long addr, free_node_t *free_node) {
//     return dma_read(addr, (void *)free_node, 1);
// }

static int read_file_node(unsigned long addr, file_node_t *file_node) {
    return dma_read(addr, (void *)file_node, 1);
}

static int read_data_node(unsigned long addr, data_node_t *data_node) {
    return dma_read(addr, (void *)data_node, 1);
}

static int get_file_node(char *filename, file_node_t *file_node) {
    superblock_t *superblock = malloc(sizeof(superblock_t));
    if (read_superblock(superblock) < 0)
        return -1;

    int addr = superblock->file_node;
    while (addr != 0) {
        if (read_file_node(addr, file_node) < 0)
            return -2;
        if (!strncmp(file_node->filename, filename, MAX_EXECNAME_LEN))
            return 0;
        addr = file_node->next;
    }

    return -3;
}

static int ls(char *buf, int count) {
    superblock_t superblock;
    if (read_superblock(&superblock) < 0)
        return -1;

    int read_len = 0;

    int addr = superblock.file_node;
    while (addr != 0) {
        file_node_t file_node;
        if (read_file_node(addr, &file_node) < 0)
            return -2;
        int len = MIN(strlen(file_node.filename) + 1, count - read_len);
        memcpy(buf + read_len, file_node.filename, len);
        read_len += len;
        if (read_len == count)
            break;
        addr = file_node.next;
    }

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
    while (addr != 0) {
        if (read_data_node(addr, data_node) < 0) {
            free(kernel_buf);
            free(file_node);
            free(data_node);
            return -5;
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
            char tmp_buf[IDE_SECTOR_SIZE];
            if (dma_read(sector, tmp_buf, 1) < 0) {
                free(kernel_buf);
                free(file_node);
                free(data_node);
                return -6;
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
            int sector_len = MIN((count - read_len) / IDE_SECTOR_SIZE, data_node->len);
            if (dma_read(sector, kernel_buf + read_len, sector_len) < 0) {
                free(kernel_buf);
                free(file_node);
                free(data_node);
                return -7;
            }
            read_len += sector_len * IDE_SECTOR_SIZE;
            sector += sector_len;
        }

        // Read last sector
        if (count - read_len > 0 &&
            sector < data_node->start + data_node->len) {
            char tmp_buf[IDE_SECTOR_SIZE];
            if (dma_read(sector, tmp_buf, 1) < 0) {
                free(kernel_buf);
                free(file_node);
                free(data_node);
                return -8;
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

    memcpy(buf, kernel_buf, read_len);
    free(kernel_buf);

    return read_len;
}

int sizefile(char *filename)
{
    file_node_t *file_node = malloc(sizeof(file_node_t));
    if (get_file_node(filename, file_node) < 0) {
        return -1;
    }

    return file_node->size;
}

int writefile(char *filename, char *buf, int count, int offset, int create)
{
    return -1;
}

int deletefile(char *filename)
{
    return -1;
}