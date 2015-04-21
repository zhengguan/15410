/** @file apic.c
 *  @brief Code for manipulating the local apic
 *
 *  @author Ryan Pearl (rpearl)
 */

#include <assert.h>
#include <smp/smp.h>
#include <smp/mptable.h>
#include <smp/apic.h>

extern int cpu2apic[MAX_CPUS];
extern int apic2cpu[MAX_CPUS];

/** @brief Read from a local APIC register
 *
 *  @param reg The register offset to read from.
 *  @return The value of the register.
 */
uint32_t lapic_read(int reg) {
	return *(volatile uint32_t *)(LAPIC_VIRT_BASE + reg);
}

/** @brief Write to a local APIC register
 *
 * @param reg The register offset to write to.
 * @param data The data to write
 * @return unit
 */
void lapic_write(int reg, uint32_t data) {
	*(volatile uint32_t *)(LAPIC_VIRT_BASE + reg) = data;
}

/** @brief Read from the local APIC before it is mapped into virtual memory.
 *  (It is almost certainly not necessary to call this function)
 *
 *  @param reg The register offset to write to.
 *  @return The data at that register.
 */
uint32_t lapic_phys_read(int reg) {
	return *(volatile uint32_t *)(smp_lapic_base() + reg);
}

/** @brief Write to the local APIC before it is mapped into virtual memory.
 *  (It is almost certainly not necessary to call this function)
 *
 *  @param reg The register offset to write to.
 *  @param data The data to write
 *  @return unit
 */
void lapic_phys_write(int reg, uint32_t data) {
	*(volatile uint32_t *)(smp_lapic_base() + reg) = data;
}

/** @brief Initialize the APIC.
 *
 *  @return unit
 */
void apic_init(void) {
	lapic_write(LAPIC_SVR, lapic_read(LAPIC_SVR) | LAPIC_ENABLE);
}


/** @brief Return this core's APIC ID.
 *
 *  @return The APIC's ID.
 */
int get_apic_id(void) {
	return lapic_read(LAPIC_ID) >> LAPIC_ID_SHIFT;
}

/** @brief Sends an APIC end-of-interrupt command.
 *
 *  @return unit
 */
void apic_eoi(void) {
	lapic_write(LAPIC_EOI, 0);
}

/** @brief Send an interprocessor interrupt to a particular target CPU.
 *
 *  @param cpuno The CPU to target
 *  @param vector The interrupt to send.
 *
 *  @return unit
 */
void apic_ipi_cpu(int cpuno, uint8_t vector) {
	assert(cpuno >= 0 && cpuno < MAX_CPUS);
	int apic_id = cpu2apic[cpuno];

	lapic_write(LAPIC_ICRHI, apic_id << LAPIC_ID_SHIFT);
	lapic_write(LAPIC_ICRLO, vector | LAPIC_ASSERT | LAPIC_FIELD);
}

/** @brief Send an interprocessor interrupt to all other CPUs
 *
 *  @param vector The interrupt to send.
 *
 *  @return unit
 */
void apic_ipi_others(uint8_t vector) {
	uint32_t val = vector | LAPIC_FIXED | LAPIC_ALLEXC | LAPIC_ASSERT;
	lapic_write(LAPIC_ICRLO, val);
}
