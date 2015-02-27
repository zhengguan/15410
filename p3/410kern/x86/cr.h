/** @file x86/cr.h
 *  @brief Control register definitions for x86 CPUs.
 */

#ifndef X86_CR_H
#define X86_CR_H

/* These are #defines so that they may be used in assembler. */

#define CR0_PE  (1 << 0)    /**< Protection Enable */
#define CR0_MP  (1 << 1)    /**< Monitor coProcessor (FPU) */
#define CR0_EM  (1 << 2)    /**< EMulation (FPU) */
#define CR0_TS  (1 << 3)    /**< Task Switched */
#define CR0_ET  (1 << 4)    /**< Extension Type (FPU) */
#define CR0_NE  (1 << 5)    /**< Numeric Error */
#define CR0_WP  (1 << 16)   /**< Write Protect */
    /** Alignment Mask.
     *
     * @note On to enable alignment checks.  This is not the only relevant
     *       bit for alignment checking -- see EFLAGS:AM.
     */
#define CR0_AM  (1 << 18)
#define CR0_NW  (1 << 29)   /**< Not Write-through */
#define CR0_CD  (1 << 30)   /**< Cache Disable */
#define CR0_PG  (1 << 31)   /**< PaGing */

/* CR1 is reserved and contains no flags we know about */
/* CR2 is used by #PF and contains no flags*/

#define CR3_PWT (1 << 3)    /**< Page-level Writes Transparent */
#define CR3_PCD (1 << 4)    /**< Page-level Cache Disable */

#define CR4_VME (1 << 0)    /**< Virtual-8086 Mode Extensions */
#define CR4_PVI (1 << 1)    /**< Protected-mode Virtual Interrupt */
#define CR4_TSD (1 << 2)    /**< Time Stamp Disable */
#define CR4_DE  (1 << 3)    /**< Debugging Extensions */
#define CR4_PSE (1 << 4)    /**< Page Size Extensions */
#define CR4_PAE (1 << 5)    /**< Physical Address Extension */
#define CR4_MCE (1 << 6)    /**< Machine Check Enable */
#define CR4_PGE (1 << 7)    /**< Page Global Enable */
#define CR4_PCE (1 << 8)    /**< Performance-monitoring Counter Enable */
#define CR4_OSFXSR (1 << 9) /**< Operating System Support FXSAVE/FXSTOR */
    /** Operating System Support for Unmasked SIMD Floating-Point Exceptions
     *
     * @note Intel's naming convention has gotten out of hand.
     */
#define CR4_OSXMMEXCPT (1 << 10)
#define CR4_VMXE (1 << 13)  /**< Virtual-Machine eXtensions Enable */
#define CR4_SMXE (1 << 14)  /**< Safer-Mode eXtensions Enable */

#define CR8_TPL_MASK  0x0000000F  /**< Task Priority Level (64-bit mode) */

#ifndef ASSEMBLER

#include <stdint.h>

/** Get CR0.
 *
 * @note Use rarely.
 */
uint32_t get_cr0();

/** Get CR2
 *
 * @note Use when appropriate.
 */
uint32_t get_cr2();

/** Get CR3, the root of the paging hierarchy.
 *
 * @note Use when appropriate.
 */
uint32_t get_cr3();

/** Get CR4.
 *
 * @note Use rarely.
 */
uint32_t get_cr4();

/** Set CR0.
 *
 * @note Use very rarely.
 */
void set_cr0(uint32_t);

/** Set CR2.
 *
 * @note Use very rarely or not at all.
 */
void set_cr2(uint32_t);

/** Set CR3.
 *
 * @note This operation implicitly flushes the TLB.
 */
void set_cr3(uint32_t);

/** Set CR4.
 *
 * @note Use very rarely.
 */
void set_cr4(uint32_t);

/*
 * ESP0 is not a register.  But we can pretend!
 */

/** Get ESP0.
 *
 * @note Use when appropriate.
 */
uint32_t get_esp0(void);

/** Set ESP0.
 *
 * @note Use when appropriate.
 */
void set_esp0(uint32_t);

#endif  /* ! ASSEMBLER */

#endif  /* ! X86_CR_H */
