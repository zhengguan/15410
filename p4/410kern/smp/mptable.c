/** @file mptable.c
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

#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <stddef.h>


#include <multiboot.h>

#include <smp/smp.h>
#include <smp/mptable.h>
#include <smp/apic.h>

#include <compiler.h>

static int num_cpus = 1;         /**< The number of CPUs */
static int bsp_id;               /**< The cpu number for the BSP */
static int bsp_apic;             /**< The APIC id for the BSP */
static void *lapic_base = NULL;  /**< Location in physical memory of the Local
                                      APIC base */
static void *ioapic_addr = NULL; /**< Location in physical memory of the I/O
                                      APIC base */

extern int cpu2apic[MAX_CPUS];
extern int apic2cpu[MAX_CPUS];

/** @brief Reset state to uniprocessor
 *
 *  @return unit
 */
static void mp_set_uniprocessor() {
	num_cpus = 1;
	ioapic_addr = NULL;
	lapic_base = NULL;
}

/** @brief Fills in the book-keeping information for a CPU
 *
 *  @param apic_id The local APIC ID for this CPU
 *  @param bsp Wether this CPU is the BSP
 *
 *  @return unit
 */
static void mp_cpu_create(int apic_id, int bsp) {

	if (num_cpus >= MAX_CPUS || apic_id >= MAX_CPUS)
		return; // out of range; ignore it.

	cpu2apic[num_cpus] = apic_id;
	apic2cpu[apic_id] = num_cpus;
	if (bsp) {
		bsp_apic = apic_id;
		bsp_id = num_cpus;
	}
	num_cpus++;
}

/** @brief Perform a checksum on an MP structure.
 *
 *  @param addr The base of the structure
 *  @param len The length of the structure
 *  @return nonzero if sucessful, 0 otherwise.
 */
static int checksum(char *addr, int len) {
	int i;
	uint8_t sum = 0;
	for (i = 0; i < len; i++) {
		sum += addr[i];
	}
	return (sum == 0);
}

/**
 * @brief Search for the MP floating pointer signature.
 *
 * @param addr The address to start searching at
 * @param len  The number of bytes to search.
 *
 * @return A pointer to the structure
 */
static struct mpfps *mp_scan_sig(uintptr_t addr, int len) {

	char *ptr;
	char *end = (char *)addr + len;

	for (ptr = (char *)addr; ptr < end; ptr += sizeof(struct mpfps)) {
		uint32_t sig = *(uint32_t *)ptr;
		if (sig == MP_FPS_SIG && checksum(ptr, sizeof(struct mpfps)))
			return (struct mpfps *)ptr;
	}
	return NULL;
}

/** @brief Walk the MP configuration table and gather data.
 *
 *  @param cfg The configuration table to read.
 *  @return unit
 */
static void mptable_walk(struct mptable *cfg) {
	num_cpus = 0; // mp_cpu_create() will count the BSP
	STATIC_ASSERT(sizeof(struct mp_cpu) == 20);
	STATIC_ASSERT(sizeof(struct mp_ioapic) == 8);
	uint8_t *ptr = (uint8_t *)(cfg + 1);
	int smp = 1;
	uint16_t i;
	for (i = 0; i < cfg->entries; i++) {
		switch (*ptr) {
			case MP_PROC: {
				struct mp_cpu *cpu = (struct mp_cpu *)ptr;
				if (cpu->flags & MP_PROC_EN) {
					mp_cpu_create(cpu->apic_id, cpu->flags & MP_PROC_BSP);
				}
				ptr += sizeof(struct mp_cpu);
				break;
			}
			case MP_IOAPIC: {
				if (ioapic_addr != NULL) {
					/* More complete code is needed to handle multiple I/O
					 * APICS. But, we don't even use the one we find. */
					smp = 0;
				} else {
					struct mp_ioapic *ioapic = (struct mp_ioapic *)ptr;
					ioapic_addr = ioapic->phys_addr;
				}
				ptr += sizeof(struct mp_ioapic);
				break;
			}
			case MP_BUS:
			case MP_IOINTR:
			case MP_LINTR:
				/* On a more complete system with actual devices, we need to
				 * care about these things.
				 */
				ptr += 8;
				break;
			default:
				panic("Unknown MP configuration entry: %x", *ptr);
		}
	}
	if (!smp) {
		mp_set_uniprocessor();
	}
}

/** @brief Find the EBDA from the BIOS.
 *
 *  @return The EBDA base.
 */
static inline uint32_t get_ebda(void) {
	uint32_t ebda_seg = (uint32_t)*(uint16_t *)MP_BIOS_EBDA;
	return ebda_seg << 4;
}

/** @brief Reads MP Table Configuration and determines whether the system has
 *         multiple processors.
 *
 *  @param info The multiboot info provided by the boatloader
 *  @return zero if multiprocessor, negative error code if not.
 */
int smp_init(mbinfo_t *info) {
	struct mpfps *fp;
	struct mptable *cfg;
	uint32_t ebda = get_ebda();
	if (ebda) {
		if ((fp = mp_scan_sig(ebda, 1024)) != NULL)
			goto found;
	}

	if ((fp = mp_scan_sig(info->mem_lower * 1024, 1024)) != NULL)
		goto found;

	if ((fp = mp_scan_sig(MP_BIOS_START, MP_BIOS_LEN)) != NULL)
		goto found;

	return -1;

	found:
	if (fp->type != 0) {
		// These are default configurations.
	 	if (fp->type != 5 && fp->type != 6)
	 		return -1; // Unsupported configuration (old hardware)

		lapic_base = (void *)LAPIC_DEFAULT_BASE;
		ioapic_addr = (void *)IOAPIC_DEFAULT_BASE;
		mp_cpu_create(0, 1);
		mp_cpu_create(1, 0);
	} else {
		cfg = (struct mptable *)fp->phys_addr;

		if (!checksum((char *)cfg, cfg->length) || cfg->sig != MP_CFG_SIG)
			return -1;

		lapic_base = (void *)cfg->lapic_addr;
		mptable_walk(cfg);
	}

	return 0;
}

/** @brief Returns the number of CPUs in the system.
 */
int smp_num_cpus(void) {
	return num_cpus;
}

/** @brief Return the base of the local APIC in physical memory.
 */
void *smp_lapic_base(void) {
	return lapic_base;
}
