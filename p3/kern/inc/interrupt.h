/** @file interrupt.h
 *  @brief Prototypes for IDT entries.
 *
 *  This contains prototypes for IDT entries.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include <stdint.h>

/* Flags for an IDT entry */
#define IDT_INT 0x06
#define IDT_TRAP 0x07
#define IDT_D 0x08
#define IDT_DPL_KERNEL 0x00
#define IDT_DPL_USER 0x60
#define IDT_P 0x80

#define TIMER_IDT_ENTRY 0x20
#define KEY_IDT_ENTRY 0x21

/* IDT functions */
void idt_init();
void idt_add_desc(int idt_entry, void *handler, unsigned gate_type, unsigned dpl);

#endif /* _INTERRUPT_H */
