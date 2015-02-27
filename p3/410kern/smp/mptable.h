/** @file mptable.h
 *
 * @brief Code to parse the MP table and determine information
 * about the APIC.
 *
 * @author Ryan Pearl (rpearl)
 *
 * This code is directly based on FreeBSD's MP table parsing code. License and
 * copyright information repeated below.
 */

/*-
 * Copyright (c) 2003 John Baldwin <jhb@FreeBSD.org>
 * Copyright (c) 1996, by Steve Passe
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. The name of the developer may NOT be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#ifndef _MPTABLE_H_
#define _MPTABLE_H_

#include <multiboot.h> // mbinfo_t
#include <stdint.h>

#define MP_FPS_SIG     0x5f504d5f   /**< The MP floating pointer signature */
#define MP_CFG_SIG     0x504d4350   /**< MP Table signature */

/** @brief Memory layout defines */
#define MP_BIOS_START 0xf0000   /**< the start of the BIOS ROM area */
#define MP_BIOS_LEN   0x10000   /**< the length of the BIOS ROM area */
#define MP_BIOS_EBDA  0x40e     /**< the location within the BIOS data area of
                                     the EBDA base */


/** @brief MP table defines */
#define MP_PROC       0x0       /**< a processor entry in the MP table */
#define MP_BUS        0x1       /**< a bus entry in the MP table (unused in 15-410) */
#define MP_IOAPIC     0x2       /**< an I/O APIC entry in the MP table */
#define MP_IOINTR     0x3       /**< an I/O interrupt entry in the MP table
                                     (unused in 15-410) */
#define MP_LINTR      0x4       /**< a local interrupt entry in the MP table
                                     (unused in 15-410) */

/** @brief Processor flag defines */
#define MP_PROC_EN     0x1       /**< Processor is enabled */
#define MP_PROC_BSP    0x2       /**< Processor is the bootstrap processor */


/** @brief Layout of the MP Floating Pointer structure */
struct mpfps {
	uint8_t sig[4];
	void *phys_addr;
	uint8_t length;
	uint8_t specrev;
	uint8_t checksum;
	uint8_t type;
	uint8_t imcrp;
	uint8_t reserved[3];
};

/** @brief Layout of the MP Configuration Table */
struct mptable {
	uint32_t sig;
	uint16_t length;
	uint8_t version;
	uint8_t checksum;
	uint8_t product[20];
	uint32_t *oem_table;
	uint16_t  oem_length;
	uint16_t entries;
	void *lapic_addr;
	uint16_t ext_length;
	uint8_t ext_checksum;
	uint8_t reserved;
};

/** @brief Layout of a processor entry in the MP Configuration Table */
struct mp_cpu {
	uint8_t type;
	uint8_t apic_id;
	uint8_t version;
	uint8_t flags;
	uint8_t sig[4];
	uint32_t features;
	uint8_t reserved[8];
};

/** @brief Layout of an I/O APIC entry in the MP Configuration Table */
struct mp_ioapic {
	uint8_t type;
	uint8_t apic_id;
	uint8_t version;
	uint8_t flags;
	void *phys_addr;
};

int   smp_init(mbinfo_t *info);
int   smp_num_cpus(void);
void *smp_lapic_base(void);

#endif
