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
#include <ide.h>

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
int block_flag = 1;
static int dma_rv = 0;

int dma_init() {
    if (mutex_init(&dma_mutex) < 0)
        return -1;

    bus_master_base = pci_find_bus_master_base();

    return 0;
}

void ide_block() {
    blocked_tcb = gettcb();
    block_flag = 0;

    // Spin-wait if deschedule fails
    while (deschedule_kern(&block_flag, false) < 0);
}

void ide_unblock() {
    block_flag = 1;
    make_runnable_kern(blocked_tcb, false);
}

void ide_interrupt_handler()
{
    int ide_status = inb(IDE_STATUS);
    int bm_status = inb(bus_master_base + IDE_BM_STATUS);

    int bm_int = bm_status & BM_STAT_INT;
    int bm_active = bm_status & BM_STAT_ACTIVE;
    
    // Ignore if no transfer started or transfer in progress
    if (!block_flag && (bm_int || !bm_active)) {
        // DMA error, Interrupt 0 and Active 0
        if (!bm_int) {
            dma_rv = -1;
        } else {
            // IDE device is busy or error
            if ((ide_status & IDE_STATUS_BUSY) ||
                (ide_status & IDE_STATUS_ERROR)) {
                dma_rv = -2;
            // Transfer successful
            } else {
                dma_rv = 0;
            }
        }

        outb(bus_master_base + IDE_BM_COMMAND, bm_status & ~BM_COM_START_STOP);

        ide_unblock();
    }

    pic_acknowledge(IDE_IRQ);
}

int dma_read(unsigned long addr, void *buf, int count)
{
    if ((unsigned)buf >= USER_MEM_START)
        return -1;

    if (!ide_present() || (addr + count > ide_size()))
        return -2;

    mutex_lock(&dma_mutex);

    if (lba_setup(addr, count, ide_lba48_enabled) < 0) {
        mutex_unlock(&dma_mutex);
        return -3;
    }

    disable_interrupts();

    prd_t prd = {
        .addr = (unsigned)buf,
        .count = count * IDE_SECTOR_SIZE,
        .flags = PRD_EOT
    };
    outd(bus_master_base + IDE_BM_PRDT, (unsigned)&prd);

    outb(bus_master_base + IDE_BM_COMMAND, BM_COM_RD_WR);

    int bm_status = inb(bus_master_base + IDE_BM_STATUS);
    outb(bus_master_base + IDE_BM_STATUS,
        bm_status |  BM_STAT_INT | BM_STAT_ERR);

    outb(IDE_COMMAND, IDE_COMMAND_READ_DMA);

    outb(bus_master_base + IDE_BM_COMMAND, BM_COM_RD_WR | BM_COM_START_STOP);

    ide_block();
    int rv = dma_rv;

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
    int rv = dma_rv;

    mutex_unlock(&dma_mutex);

    return rv;
}
