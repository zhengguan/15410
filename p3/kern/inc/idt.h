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
#define IDT_TRAP 0x0F
#define IDT_INT 0x0F
#define IDT_D 0x0F
#define IDT_DPL 0x00
#define IDT_P 0x80

#define TIMER_IDT_ENTRY 0x20
#define KEY_IDT_ENTRY 0x21

typedef void (*idt_handler_t)(void);

/* An IDT entry */
typedef struct idt_desc {
    uint16_t offset_l;
    uint16_t segment_selector;
    uint8_t zeros;
    uint8_t flags;
    uint16_t offset_h;
} idt_desc_t;

#endif /* _IDT_H */
