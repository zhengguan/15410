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
// TODO remove this
#include <loader.h>
#include <simics.h>

#define SUPERBLOCK_ADDR 0

typedef struct superblock {
    int constant;
    int file_node;
    int free_node;
    char padding[IDE_SECTOR_SIZE - 3*sizeof(int)];
} superblock_t;

typedef struct free_node {
    int next;
    int extent_len;
    char padding[IDE_SECTOR_SIZE - 2*sizeof(int)];
} free_node_t;

typedef struct file_node {
    int next;
    char filename[MAX_EXECNAME_LEN];
    int size;
    int writeable;
    int data_node;
    char padding[IDE_SECTOR_SIZE - 4*sizeof(int) -
                 MAX_EXECNAME_LEN*sizeof(char)];
} file_node_t;

typedef struct data_node {
    int next;
    int extent_len;
    int extent;
    char padding[IDE_SECTOR_SIZE - 3*sizeof(int)];
} data_node_t;

static int read_superblock(superblock_t *superblock) {
    return dma_read(SUPERBLOCK_ADDR, (void *)superblock, 1);
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
    superblock_t superblock;
    if (read_superblock(&superblock) < 0) 
        return -1;

    int addr = superblock.file_node;
    while (addr != 0) {
        if (read_file_node(addr, file_node) < 0)
            return -2;
        if (!strncmp(file_node->filename, filename, MAX_EXECNAME_LEN))
            return 0;
        addr = file_node->next;
    }

    return -3;
}

int readfile(char *filename, char *buf, int count, int offset)
{
    char *kernel_buf[count];

    lprintf("Attempting to read '%s' at %p for %d", filename, kernel_buf, count);

    file_node_t file_node;
    if (get_file_node(filename, &file_node) < 0)
        return -1;

    lprintf("Found file");

    int read_len = 0;

    int addr = file_node.data_node;
    while (addr != 0) {
        data_node_t data_node;
        if (read_data_node(addr, &data_node) < 0)
            return -2;

        int sector = data_node.extent;
        int sector_offset = offset / IDE_SECTOR_SIZE;
        int extent_bytes = data_node.extent_len * IDE_SECTOR_SIZE;

        // Skip entire extent
        if (sector_offset >= data_node.extent_len) {
            offset -= extent_bytes;
            continue;
        }

        // Read first sector
        if (offset > 0) {
            sector += sector_offset;
            char tmp_buf[IDE_SECTOR_SIZE];
            if (dma_read(sector, tmp_buf, 1) < 0)
                return -3;
            int len = MIN(count, IDE_SECTOR_SIZE - offset);
            memcpy(kernel_buf, tmp_buf + offset, len);
            read_len += len;
            sector++;
        }

        // Read sectors
        if (count - read_len >= IDE_SECTOR_SIZE) {
            int sector_len = (count - read_len) / IDE_SECTOR_SIZE;
            if (dma_read(sector, kernel_buf + read_len, sector_len) < 0)
                return -3;
            read_len += extent_bytes;
            sector += sector_len;
        }

        // Read last sector
        if (count - read_len > 0 &&
            sector < data_node.extent + data_node.extent_len) {
            char tmp_buf[IDE_SECTOR_SIZE];
            if (dma_read(sector, tmp_buf, 1) < 0)
                return -3;
            int len = MIN(count, IDE_SECTOR_SIZE - offset);
            memcpy(kernel_buf, tmp_buf + offset, len);
            read_len += len;
            sector++;
        }

        if (read_len == count)
            break;

        addr = data_node.next;
    }

    lprintf("Readfile returned %d", read_len);
    memcpy(buf, kernel_buf, read_len);
    return read_len;
}

int sizefile(char *filename)
{    
    file_node_t file_node;
    if (get_file_node(filename, &file_node) < 0)
        return -1;

    return file_node.size;
}

int writefile(char *filename, char *buf, int count, int offset, int create)
{
    return -1;
}

int deletefile(char *filename)
{
    return -1;
}