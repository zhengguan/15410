/**
 *  @file smp.c
 *  @brief SMP bootup and utility code.
 *
 *  @author Ryan Pearl (rpearl)
 *  @author Michael Sullivan (mjsulliv)
 */

#include <assert.h>
#include <string.h>
#include <smp/smp.h>
#include <smp/smpboot.h>
#include <smp/mptable.h>
#include <smp/apic.h>

#include <x86/timer_defines.h>
#include <x86/asm.h>
#include <x86/seg.h>
#include <x86/idt.h>
#include <x86/cr.h>
#include <x86/page.h>


extern uint64_t init_gdt[GDT_SEGS];
extern char *init_tss;

extern uint64_t tss_desc_create(void *tss, size_t tss_size);

char smp_boot_stack[INIT_STACK_SIZE][MAX_CPUS];

extern uint32_t smp_boot_gdt_base;
extern uint16_t smp_boot_gdt_limit;
static void *smp_idt_base; /**< Stores the 32-bit IDT base before booting the
                                other cores. */

/** @brief a mapping from APIC IDs to CPU numbers */
int apic2cpu[MAX_CPUS];
/** @brief a mapping from CPU numbers to APIC IDs */
int cpu2apic[MAX_CPUS];

volatile int cpus_booted = 1;
static void (*smp_main_fn)(int cpu); /**< Stores the entry point function for
									   the APs to run. */


/**
 * @brief Return what the kernel considers the CPU number of the current CPU
 * Note that the CPU number is generally only safe to use if interrupts
 * are disabled. Otherwise, the thread could get preempted and scheduled
 * in the meantime.
 *
 * @return The current cpu number
 */
int smp_get_cpu(void) {
	return apic2cpu[get_apic_id()];
}

/**
 * @brief The first function run in C, in protected mode.
 *        Sets up the CPU state and calls the SMP entrypoint function.
 * @return Does not return.
 */

void squidboy(void) {
	/* paging is not enabled, so read from physical memory. */
	int apic_id = (int)lapic_phys_read(LAPIC_ID);
	apic_id = apic_id >> LAPIC_ID_SHIFT;

	int cpuno = apic2cpu[apic_id];

	/* enable local APIC */
	lapic_phys_write(LAPIC_SVR, lapic_phys_read(LAPIC_SVR) | LAPIC_ENABLE);
	/* clear error status register. Requires two writes */
	lapic_phys_write(LAPIC_ESR, 0);
	lapic_phys_write(LAPIC_ESR, 0);

	/* clear interrupts */
	lapic_phys_write(LAPIC_EOI, 0);

	/* This cpu is currently running on the boot GDT: we needed to get to
	 * protected mode. Now, since the GDT contains a TSS entry, and the TSS is
	 * responsible for maintaining the esp0 "register" (among other things) we
	 * switch to a separate GDT and TSS for this core.
	 */
	extern const size_t init_tss_size;
	char tss[init_tss_size];
	memcpy(&tss, &init_tss, init_tss_size);

	uint64_t gdt[GDT_SEGS];
	memcpy(gdt, init_gdt, sizeof(init_gdt));

	uint64_t tss_desc;
	tss_desc = tss_desc_create(tss, init_tss_size);

	gdt[SEGSEL_KERNEL_TSS_IDX] = tss_desc;
	lgdt(gdt, sizeof(gdt)-1);


	/* Load the same IDT into this GDT */
	lidt(smp_idt_base, IDT_ENTS * IDT_ENT_SIZE);

	/* Reload task register */
	ltr(SEGSEL_TSS);

	/* Disable floating point unit:
	 * see 410kern/boot/entry.c
	 */
	set_cr0(get_cr0() | CR0_EM);

	smp_main_fn(cpuno);
	panic("cpu %d: smp entrypoint returned.", cpuno);
}

/** @brief Delay for a number of microseconds.
 *
 *  @return unit
 */
static void udelay(int us) {
	int i;
	for (i = 0; i < us; i++)
		iodelay();
}

static void apic_wait() {
	while ((lapic_read(LAPIC_ICRLO) & LAPIC_DELIVS) != 0)
		continue;
}

static int smp_boot_cpu(int cpuno) {
	uint8_t entry = SMP_INIT_PAGE >> PAGE_SHIFT;
	int apic_id = cpu2apic[cpuno];
	int booted = cpus_booted;

	/* send INIT IPI to this CPU */
	lapic_write(LAPIC_ICRHI, apic_id << LAPIC_ID_SHIFT);
	lapic_write(LAPIC_ICRLO, LAPIC_INIT | LAPIC_ASSERT | LAPIC_FIELD);
	udelay(1000 * 10); // 10ms
	apic_wait();

	lapic_write(LAPIC_ICRHI, apic_id << LAPIC_ID_SHIFT);
	lapic_write(LAPIC_ICRLO, entry | LAPIC_STARTUP | LAPIC_FIELD | LAPIC_DEASSERT);
	apic_wait();
	udelay(200); // 20us

	lapic_write(LAPIC_ICRHI, apic_id << LAPIC_ID_SHIFT);
	lapic_write(LAPIC_ICRLO, entry | LAPIC_STARTUP | LAPIC_FIELD | LAPIC_DEASSERT);
	apic_wait();
	udelay(200); // 20us
	int ms;
	for (ms = 0; ms < 500; ms++) {
		if (cpus_booted > booted)
			return 1;
		udelay(1000);
	}
	return 0;
}

/** @brief Sends the INIT and SIPI message to the APs to boot them.
 *
 *  @return unit
 */
static void smp_boot_cpus() {
	int cpu;
	int num_cpus = smp_num_cpus();
	for (cpu = 1; cpu < num_cpus; cpu++) {
		if (!smp_boot_cpu(cpu)) {
			panic("cpu %d did not boot\n", cpu);
		}
	}
}

/** @brief Boot the application processors
 *
 *  @param entry The entry point function. The parameter is the cpu number.
 *  @return unit
 */
void smp_boot(void (*entry)(int)) {
	smp_main_fn = entry;

	/* Gather information from the BSP that the APs need. */
	smp_boot_gdt_base = (uint32_t)&init_gdt;
	smp_boot_gdt_limit = sizeof(init_gdt) - 1;
	smp_idt_base = idt_base();

	apic_init();

	/* Copy code to a low address; it has to run in real mode. */
	extern char startup_begin;
	memcpy((void *)SMP_INIT_PAGE, &startup_begin, PAGE_SIZE);
	smp_boot_cpus();
}
