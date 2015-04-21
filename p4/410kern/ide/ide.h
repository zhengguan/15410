/** @file ide.h
 *
 *  @brief Definitions related to the IDE bus
 *
 *  @author Joshua Wise (jwise) <joshua@joshuawise.com>
 *
 *  Bus-master support added S'15 by Christopher D. Williamson
 *  and Caleb Levine.
 *
 *  @author Christopher D. Williamson (cdw1)
 *  @author Caleb J. Levine (cjlevine)
 */

#ifndef __IDE_H
#define __IDE_H

#include <x86/pic.h>
#include <stdint.h>

/* start: provided by the kernel */
extern void ide_block(void);
extern void ide_unblock(void);
extern void ide_interrupt_handler(void);
extern int dma_init(void);
extern int dma_read(unsigned long addr, void *buf, int count);
extern int dma_write(unsigned long addr, void *buf, int count);
/* end: provided by the kernel */

extern int ide_init(void);
extern int ide_present(void);
extern int ide_size(void);
extern int ide_read(unsigned long addr, void *buf, int count);
extern int ide_write(unsigned long addr, void *buf, int count);
/* Exposed for use by dma_read and dma_write */
int lba_setup(uint64_t addr, int count, int lba48);

extern void ide_interrupt_handler();

#define IDE_SECTOR_SIZE (512)
#define IDE_IRQ (0xE)
#define IDE_IDT_ENTRY	((X86_PIC_MASTER_IRQ_BASE) + IDE_IRQ)

/* from http://www.repairfaq.org/filipg/LINK/F_IDE-tech.html */
#define IDE_DATA 0x1F0
#define IDE_ERR 0x1F1
#define IDE_ERR_BBK 0x80
#define IDE_ERR_UNC 0x40
#define IDE_ERR_IDNF 0x10
#define IDE_ERR_ABRT 0x04
#define IDE_ERR_TK0NF 0x02
#define IDE_ERR_AMNF 0x01
#define IDE_FEATURES 0x1F1
#define IDE_SECCNT 0x1F2
#define IDE_SECNUM 0x1F3
#define IDE_CYLL 0x1F4
#define IDE_LBAL 0x1F4
#define IDE_CYLH 0x1F5
#define IDE_LBAM 0x1F5
#define IDE_SELECT 0x1F6
#define IDE_SELECT_DRV 0x10
#define IDE_SELECT_RSVD 0xA0
#define IDE_SELECT_LBA 0x40
#define IDE_STATUS 0x1F7
#define IDE_STATUS_BUSY 0x80
#define IDE_STATUS_DRDY 0x40
#define IDE_STATUS_DWF 0x20
#define IDE_STATUS_DSC 0x10
#define IDE_STATUS_DRQ 0x08
#define IDE_STATUS_CORR 0x04
#define IDE_STATUS_INDEX 0x02
#define IDE_STATUS_ERROR 0x01
#define IDE_COMMAND 0x1F7
#define IDE_COMMAND_IDENTIFY 0xEC
#define IDE_COMMAND_READ28 0x20
#define IDE_COMMAND_WRITE28 0x30
#define IDE_COMMAND_READ48 0x24
#define IDE_COMMAND_WRITE48 0x34
#define IDE_COMMAND_READ_DMA 0xC8
#define IDE_COMMAND_WRITE_DMA 0xCA
#define IDE_DCR 0x3F6
#define IDE_DCR_RSVD 0x08
#define IDE_DCR_SRST 0x04
#define IDE_DCR_nIE 0x02
#define IDE_ALTSTATUS 0x3F6

/* Begin IDE bus-master DMA definitions
 * see http://www.bswd.com/idems100.pdf
 */

/* Bus master control register locations (offset from base address) */
#define IDE_BM_COMMAND 0x00
#define IDE_BM_STATUS 0x02
#define IDE_BM_PRDT 0x04

/* Masks for fields in bus master control registers */
#define BM_STAT_ACTIVE  0x01
#define BM_STAT_ERR     0x02
#define BM_STAT_INT     0x04
#define BM_STAT_DMA0    0x20
#define BM_STAT_DMA1    0x40
#define BM_STAT_SIMP    0x80

#define BM_COM_START_STOP   0x01
#define BM_COM_RD_WR        0x08

#define RES_EOT (1 << 15)

typedef struct {
    int base;
    unsigned short count;
    unsigned short reserved_eot;
} prd_entry_t;
/* End IDE bus-master DMA definitions */

/** @brief Offset into the MBR for the partition table. */
#define HD_MBR_PTAB 0x1BE

/** @brief Partition type for PebblesFS. */
#define HD_PARTITION_PEBBLES 0xAA

#endif
