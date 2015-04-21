/** @file 410kern/inc/x86/eflags.h
 *  @brief Includes symbolic names for the EFLAGS condition register.
 *
 *  @note All bits above EFL_ID (0x00100000) are Reserved, Maintain 0.
 */

#ifndef X86_EFLAGS_H
#define X86_EFLAGS_H

/* These are #define rather than an enum so that they may be used
 * from assembler. */

#define EFL_CF          0x00000001      /**< Carry Flag */
#define EFL_RESV1       0x00000002      /**< Reserved, Maintain 1 */
#define EFL_PF          0x00000004      /**< Parity Flag */
#define EFL_RESV2       0x00000008      /**< Reserved, Maintain 0 */
#define EFL_AF          0x00000010      /**< Auxiliary Flag */
#define EFL_RESV3       0x00000020      /**< Reserved, Maintain 0 */
#define EFL_ZF          0x00000040      /**< Zero Flag */
#define EFL_SF          0x00000080      /**< Sign Flag */
#define EFL_TF          0x00000100      /**< Trap Flag (single step) */
#define EFL_IF          0x00000200      /**< Interrupt Flag */
#define EFL_DF          0x00000400      /**< Direction Flag */
#define EFL_OF          0x00000800      /**< Overflow Flag */
#define EFL_IOPL_RING0  0x00000000      /**< IO only at ring 0 */
#define EFL_IOPL_RING1  0x00001000      /**< IO only at ring 0/1 */
#define EFL_IOPL_RING2  0x00002000      /**< IO only at ring 0/1/2 */
#define EFL_IOPL_RING3  0x00003000      /**< IO for all and sundry */
#define EFL_IOPL_SHIFT  12              /**< Leftshift for IOPL location */
#define EFL_NT          0x00004000      /**< Nested Task */
#define EFL_RESV4       0x00008000      /**< Reserved, Maintain 0 */
#define EFL_RF          0x00010000      /**< Resume Flag */

    /** Virtual 8086 Mode
     *
     * @warning THIS IS NOT RELATED to Virtual Memory "VM".  Unless you
     *          are certain, this bit is not the bit you are looking for.
     */
#define EFL_VM          0x00020000

    /** Alignment Check
     *
     * @note This bit is modifiable by user code.  See CR0:AM for another.
     */
#define EFL_AC          0x00040000

#define EFL_VIF         0x00080000      /**< Virtual Interrupt Flag */
#define EFL_VIP         0x00100000      /**< Virtual Interrupt Pending */
#define EFL_ID          0x00200000      /**< IDentification Support (CPUID) */

#ifndef ASSEMBLER

#include <stdint.h>

uint32_t get_eflags(void);
void set_eflags(uint32_t);

#endif

#endif /* !X86_EFLAGS_H */
