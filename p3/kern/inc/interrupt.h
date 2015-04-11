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

#define TIMER_IDT_ENTRY 32
#define KEY_IDT_ENTRY 	33

/* IDT functions */
void idt_init();
void idt_add_desc(int idt_entry, void *handler, unsigned gate_type, unsigned dpl);

#endif /* _INTERRUPT_H */
