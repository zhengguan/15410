/** @file idt.h
 *  @brief Prototypes for IDT entries.
 *
 *  This contains prototypes for IDT entries.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _IDT_H_
#define _IDT_H_

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

/* An IDT entry */
typedef struct idt_desc {
    uint16_t offset_l;
    uint16_t segment_selector;
    uint8_t zeros;
    uint8_t flags;
    uint16_t offset_h;
} idt_desc_t;

/* IDT functions */
void idt_init();
void idt_add_desc(int idt_entry, void *handler, unsigned gate_type, unsigned dpl);

#endif /* _IDT_H */
