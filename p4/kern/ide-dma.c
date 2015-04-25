/** @file dma.c
 *  @brief Interface between Pebbles kernel and the 410 IDE driver.
 *
 *  Please refer to the P4 handout.
 *
 *  @author Caleb Levine (cjlevine)
 *  @author Chris Williamson (cdw1)
 */

#include <stdlib.h>
#include <asm.h>
#include <ide.h>
#include <pci.h>
#include <ide-config.h>
#include <mutex.h>
#include <scheduler.h>
#include <simics.h>

#define PRD_EOT 0x8000

typedef struct prd {
    uint32_t addr;
    uint16_t count;
    uint16_t flags;    
} prd_t;

static int ide_lba48_enabled = 0;
static int bus_master_base;

static mutex_t dma_mutex;
static tcb_t *blocked_tcb;
static int dma_status = 0;

int dma_init() {
    if (mutex_init(&dma_mutex) < 0)
        return -1;

    bus_master_base = pci_find_bus_master_base();

    // TODO is this necessary?
    int bm_status = inb(bus_master_base + IDE_BM_STATUS);
    outb(bus_master_base + IDE_BM_STATUS,
        bm_status |  BM_STAT_INT | BM_STAT_ERR);

    return 0;
}

void ide_block() {
    blocked_tcb = gettcb();
    int block_flag = 0;
    deschedule_kern(&block_flag, false);
    enable_interrupts();
}

void ide_unblock() {
    make_runnable_kern(blocked_tcb, false);
}

void ide_interrupt_handler()
{
    int bm_status = inb(bus_master_base + IDE_BM_STATUS);

    if (!(bm_status & BM_STAT_INT)) {
        if (!(bm_status & BM_STAT_ACTIVE)) {
            // An error has occured
            // TODO should we retry? see idems100.pdf
            dma_status = -1;
        }
    } else {
        dma_status = 0;
        ide_unblock();
    }

    pic_acknowledge(IDE_IRQ);
}

int dma_read(unsigned long addr, void *buf, int count)
{
    // TODO check that buf is in kernel

    // TODO Why do we need to do this??
    inb(IDE_STATUS);

    if (!ide_present() || (addr + count > ide_size()))
        return -1;

    lprintf("DMA read from addr %lu into buf %p, for count %d", addr, buf, count);

    mutex_lock(&dma_mutex);

    if (lba_setup(addr, count, ide_lba48_enabled) < 0) {
        mutex_unlock(&dma_mutex);
        return -2;
    }

    disable_interrupts();

    // TODO fix masking
    prd_t prd = {
        .addr = (unsigned)buf,
        .count = count * IDE_SECTOR_SIZE,
        .flags = PRD_EOT
    };
    outd(bus_master_base + IDE_BM_PRDT, (unsigned)&prd & ~0x3);

    int bm_status = inb(bus_master_base + IDE_BM_STATUS);
    outb(bus_master_base + IDE_BM_STATUS,
        bm_status |  BM_STAT_INT | BM_STAT_ERR);

    outb(IDE_COMMAND, IDE_COMMAND_READ_DMA);

    outb(bus_master_base + IDE_BM_COMMAND, BM_COM_RD_WR | BM_COM_START_STOP);

    ide_block();
    int rv = dma_status;

    mutex_unlock(&dma_mutex);

    return rv;
}

int dma_write(unsigned long addr, void *buf, int count)
{
    if (!ide_present() || (addr + count > ide_size()))
        return -1;

    if (lba_setup(addr, count, ide_lba48_enabled) < 0)
        return -2;

    mutex_lock(&dma_mutex);

    disable_interrupts();

    int bm_status = inb(bus_master_base + IDE_BM_STATUS);
    outb(bus_master_base + IDE_BM_STATUS,
        bm_status |  BM_STAT_INT | BM_STAT_ERR);
    outb(bus_master_base + IDE_BM_COMMAND, BM_COM_START_STOP);

    ide_block();
    int rv = dma_status;

    mutex_unlock(&dma_mutex);

    return rv;
}
