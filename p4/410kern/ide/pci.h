/** @file pci.h
 *
 *  @brief Definitions related to PCI hardware
 *
 *  Copyright (C) 2003, Lucent Technologies Inc. and others. All Rights Reserved.
 *  See PCI_LICENSE in this directory.
 *
 *  @author Chris Williamson (cdw1)
 *  @author Caleb Levine (cjlevine)
 */

#ifndef _PCI_H_
#define _PCI_H_

int pci_read_word(int dev_info, int reg);
int pci_find_device(int dev_class);
int pci_find_bus_master_base(void);

/* Bitshuffling macros and definitions shamelessly taken from
 * http://plan9.bell-labs.com/sources/plan9/sys/src/9/pc/pci.c
 */
#define PCI_DATA 0xCFC
#define PCI_ADDR 0xCF8

#define PCI_BUS_BASE    0x80000000

#define MKBUS(t,b,d,f)  (((t)<<24)|(((b)&0xFF)<<16)|(((d)&0x1F)<<11)|(((f)&0x07)<<8))
#define BUSFNO(tbdf)    (((tbdf)>>8)&0x07)
#define BUSDNO(tbdf)    (((tbdf)>>11)&0x1F)
#define BUSBNO(tbdf)    (((tbdf)>>16)&0xFF)
#define BUSTYPE(tbdf)   ((tbdf)>>24)
#define BUSBDF(tbdf)    ((tbdf)&0x00FFFF00)

/* PCI bus, device, and function are all indexed 0-7 */
#define PCI_NUM_BUS     8
#define PCI_NUM_DEV     8
#define PCI_NUM_FUN     8

#define PCI_CLASS_OFFSET    16
#define PCI_CLASS_IDE       0x0101
#define BM_BASE_ADDR_MASK   0xFFFFFFFC

enum {
    PCI_VID      = 0x00,     /* vendor ID */
    PCI_DID      = 0x02,     /* device ID */
    PCI_PCR      = 0x04,     /* command */
    PCI_PSR      = 0x06,     /* status */
    PCI_RID      = 0x08,     /* revision ID */
    PCI_CCRP     = 0x09,     /* programming interface class code */
    PCI_CCRU     = 0x0A,     /* sub-class code */
    PCI_CCRB     = 0x0B,     /* base class code */
    PCI_CLS      = 0x0C,     /* cache line size */
    PCI_LTR      = 0x0D,     /* latency timer */
    PCI_HDT      = 0x0E,     /* header type */
    PCI_BST      = 0x0F,     /* BIST */

    PCI_BAR0     = 0x10,     /* base address */
    PCI_BAR1     = 0x14,

    PCI_BMBASE   = 0x20,     /* Bus Master base address */

    PCI_INTL     = 0x3C,     /* interrupt line */
    PCI_INTP     = 0x3D,     /* interrupt pin */
};

enum {
    BUS_CBUS     = 0,        /* Corollary CBUS */
    BUS_CBUSII,          /* Corollary CBUS II */
    BUS_EISA,            /* Extended ISA */
    BUS_FUTURE,          /* IEEE Futurebus */
    BUS_INTERN,          /* Internal bus */
    BUS_ISA,             /* Industry Standard Architecture */
    BUS_MBI,             /* Multibus I */
    BUS_MBII,            /* Multibus II */
    BUS_MCA,             /* Micro Channel Architecture */
    BUS_MPI,             /* MPI */
    BUS_MPSA,            /* MPSA */
    BUS_NUBUS,           /* Apple Macintosh NuBus */
    BUS_PCI,             /* Peripheral Component Interconnect */
    BUS_PCMCIA,          /* PC Memory Card International Association */
    BUS_TC,              /* DEC TurboChannel */
    BUS_VL,              /* VESA Local bus */
    BUS_VME,             /* VMEbus */
    BUS_XPRESS,          /* Express System Bus */
};

#endif /* _PCI_H_ */
