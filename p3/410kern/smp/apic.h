/**
 * @file   apic.h
 * @brief  apic related definitions and stuff
 *
 * @author Ryan Pearl (rpearl)
 * @author Michael Sullivan (mjsulliv)
 */

#ifndef APIC_H
#define APIC_H

#define LAPIC_DEFAULT_BASE    0xfee00000
#define IOAPIC_DEFAULT_BASE   0xfec00000

#define LAPIC_VIRT_BASE        0x1000

#define LAPIC_ID_SHIFT         24


/* Local APIC registers */
#define LAPIC_ID               0x020     /* ID */
#define LAPIC_VER              0x030     /* Version */
#define LAPIC_TPR              0x080     /* Task Priority */
#define LAPIC_APR              0x090     /* Arbitration Priority */
#define LAPIC_PPR              0x0A0     /* Processor Priority */
#define LAPIC_EOI              0x0B0     /* EOI */
#define LAPIC_LDR              0x0D0     /* Logical Destination */
#define LAPIC_DFR              0x0E0     /* Destination Format */
#define LAPIC_SVR              0x0F0     /* Spurious Interrupt Vector */
#define LAPIC_ISR              0x100     /* Interrupt Status (8 registers) */
#define LAPIC_TMR              0x180     /* Trigger Mode (8 registers) */
#define LAPIC_IRR              0x200     /* Interrupt Request (8 registers) */
#define LAPIC_ESR              0x280     /* Error Status */
#define LAPIC_ICRLO            0x300     /* Interrupt Command */
#define LAPIC_ICRHI            0x310     /* Interrupt Command [63:32] */
#define LAPIC_LVT_TIMER        0x320     /* Local Vector Table 0 (TIMER) */
#define LAPIC_PCINT            0x340     /* Performance Counter LVT */
#define LAPIC_LINT0            0x350     /* Local Vector Table 1 (LINT0) */
#define LAPIC_LINT1            0x360     /* Local Vector Table 2 (LINT1) */
#define LAPIC_ERROR            0x370     /* Local Vector Table 3 (ERROR) */
#define LAPIC_TIMER_INIT       0x380     /* Timer Initial Count */
#define LAPIC_TIMER_CUR        0x390     /* Timer Current Count */
#define LAPIC_TIMER_DIV        0x3E0     /* Timer Divide Configuration */

/*
 * Common bits for
 *	I/O APIC Redirection Table Entry;
 *	Local APIC Local Interrupt Vector Table;
 *	Local APIC Inter-Processor Interrupt;
 *	Local APIC Timer Vector Table.
 */
#define LAPIC_FIXED       0x00000000     /* [10:8] Delivery Mode */
#define LAPIC_LOWEST      0x00000100     /* Lowest priority */
#define LAPIC_SMI         0x00000200     /* System Management Interrupt */
#define LAPIC_RR          0x00000300     /* Remote Read */
#define LAPIC_NMI         0x00000400
#define LAPIC_INIT        0x00000500     /* INIT/RESET */
#define LAPIC_STARTUP     0x00000600     /* Startup IPI */
#define LAPIC_EXT_INT     0x00000700

#define LAPIC_PHYSICAL    0x00000000     /* [11] Destination Mode (RW) */
#define LAPIC_LOGICAL     0x00000800

#define LAPIC_DELIVS      0x00001000     /* [12] Delivery Status (RO) */
#define LAPIC_HIGH        0x00000000     /* [13] Interrupt Input Pin Polarity (RW) */
#define LAPIC_LOW         0x00002000
#define LAPIC_REMOTE_IRR  0x00004000     /* [14] Remote IRR (RO) */
#define LAPIC_EDGE        0x00000000     /* [15] Trigger Mode (RW) */
#define LAPIC_LEVEL       0x00008000
#define LAPIC_IMASK       0x00010000     /* [16] Interrupt Mask */

/* Defines for LAPIC_SVR   */
#define LAPIC_ENABLE      0x00000100     /* Unit Enable */
#define LAPIC_FOCUS       0x00000200     /* Focus Processor Checking Disable */

/* Defines for LAPIC_IRCLO */
/* [14] IPI Trigger Mode Level (RW) */
#define LAPIC_DEASSERT    0x00000000     /* Deassert level-sensitive interrupt */
#define LAPIC_ASSERT      0x00004000     /* Assert level-sensitive interrupt */

/* [17:16] Remote Read Status */
#define LAPIC_INVALID     0x00000000     /* Invalid */
#define LAPIC_WAIT        0x00010000     /* In-Progress */
#define LAPIC_VALID       0x00020000     /* Valid */

/* [19:18] Destination Shorthand */
#define LAPIC_FIELD       0x00000000     /* No shorthand */
#define LAPIC_SELF        0x00040000     /* Self is single destination */
#define LAPIC_ALLINC      0x00080000     /* All including self */
#define LAPIC_ALLEXC      0x000C0000     /* All Excluding self */

/* Defines for LAPIC_ESR */
#define LAPIC_SENDCS      0x00000001     /* Send CS Error */
#define LAPIC_RCVCS       0x00000002     /* Receive CS Error */
#define LAPIC_SENDACCEPT  0x00000004     /* Send Accept Error */
#define LAPIC_RCVACCEPT   0x00000008     /* Receive Accept Error */
#define LAPIC_SENDVECTOR  0x00000020     /* Send Illegal Vector */
#define LAPIC_RCVVECTOR   0x00000040     /* Receive Illegal Vector */
#define LAPIC_REGISTER    0x00000080     /* Illegal Register Address */

/* Defines for LAPIC_TIMER */
/* [17] Timer Mode (RW) */
#define LAPIC_ONESHOT     0x00000000     /* One-shot */
#define LAPIC_PERIODIC    0x00020000     /* Periodic */

/* [19:18] Timer Base (RW) */
#define LAPIC_CLKIN       0x00000000     /* use CLKIN as input */
#define LAPIC_TMBASE      0x00040000     /* use TMBASE */
#define LAPIC_DIVIDER     0x00080000     /* use output of the divider */

/* Defines for LAPIC_TIMER_DIV */
#define LAPIC_X2          0x00000000     /* divide by 2 */
#define LAPIC_X4          0x00000001     /* divide by 4 */
#define LAPIC_X8          0x00000002     /* divide by 8 */
#define LAPIC_X16         0x00000003     /* divide by 16 */
#define LAPIC_X32         0x00000008     /* divide by 32 */
#define LAPIC_X64         0x00000009     /* divide by 64 */
#define LAPIC_X128        0x0000000A     /* divide by 128 */
#define LAPIC_X1          0x0000000B     /* divide by 1 */

#ifndef ASSEMBLER
#include <stdint.h>

void lapic_eoi(void);
uint32_t lapic_read(int reg);
uint32_t lapic_phys_read(int reg);
void lapic_write(int reg, uint32_t data);
void lapic_phys_write(int reg, uint32_t data);

int get_apic_id(void);

void apic_init(void);

void apic_eoi(void);
void apic_ipi_cpu(int cpuno, uint8_t vector);
void apic_ipi_others(uint8_t vector);

#endif
#endif
