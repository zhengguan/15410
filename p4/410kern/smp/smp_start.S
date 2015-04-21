/** @brief SMP bootstrap code.
 *
 *  @author Ryan Pearl (rpearl)
 */

#include <x86/seg.h>
#include <x86/cr.h>
#include <smp/smpboot.h>

/** @brief refer to a location once it's been moved. */
#define REF(loc) (loc - startup_begin + SMP_INIT_PAGE)

/** @brief 16-bit initially run on initial boot */
.code16
.global startup_begin
startup_begin:
	cli
	cld
	//get to protected mode
	lgdt REF(smp_boot_gdt_limit)
	movl $CR0_PE, %eax
	movl %eax, %cr0
	ljmp $SEGSEL_KERNEL_CS, $REF(smp_begin_protected)

/** @brief Code run once 32-bit protected mode is enabled. */
.code32
smp_begin_protected:
	/* perform an absolute jump to the smp startup code not in low-memory. */
	movl $smp_main, %eax
	jmp *%eax
smp_main:
	movl $SEGSEL_KERNEL_DS, %eax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss

	movl $1, %esi
	lock xaddl %esi, cpus_booted

	movl %esi, %ecx
	shl $INIT_STACK_SHIFT, %ecx
	addl $smp_boot_stack, %ecx
	movl %ecx, %esp
	call squidboy


/** @brief Initial GDT's limit */
.global smp_boot_gdt_limit
smp_boot_gdt_limit: .word 0

/** @brief Initial GDT's base */
.global smp_boot_gdt_base
smp_boot_gdt_base:  .long 0
