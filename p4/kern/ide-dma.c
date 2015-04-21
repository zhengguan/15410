/** @file dma.c
 *  @brief Interface between Pebbles kernel and the 410 IDE driver.
 *
 *  Please refer to the P4 handout.
 *
 *  @author Caleb Levine (cjlevine)
 *  @author Chris Williamson (cdw1)
 */

#include <stdlib.h>
#include <ide.h>
#include <pci.h>
#include <ide-config.h>
#include <simics.h>

int dma_init() {
    return -1;
}

void ide_block() {
}

void ide_unblock() {
}

void ide_interrupt_handler()
{
    pic_acknowledge(IDE_IRQ);
}

int dma_read(unsigned long addr, void *buf, int count)
{
    return -1;
}

int dma_write(unsigned long addr, void *buf, int count)
{
    return -1;
}
