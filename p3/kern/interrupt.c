/** @file interrupt.c
 *  @brief Initializes the IDT entries.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <interrupt.h>
#include <x86/idt.h>
#include <x86/asm.h>
#include <x86/asm.h>
#include <x86/seg.h>
#include <syscall_int.h>
#include <handler.h>
#include <scheduler.h>
#include <exception_asm.h>
#include <timer.h>
#include <keyboard.h>

extern int new_pages(void *base, int len);
extern int remove_pages(void *base);

/* Flags for an IDT entry */
#define IDT_INT 0x06
#define IDT_TRAP 0x07
#define IDT_D 0x08
#define IDT_DPL_KERNEL 0x00
#define IDT_DPL_USER 0x60
#define IDT_P 0x80

/* An IDT entry */
typedef struct idt_desc {
    uint16_t offset_l;
    uint16_t segment_selector;
    uint8_t zeros;
    uint8_t flags;
    uint16_t offset_h;
} idt_desc_t;

/** @brief Add a descriptor entry to the IDT.
 *
 *  @param idt_entry The IDT entry number.
 *  @param handler The interrupt handler.
 *  @param gate_type The descriptor gate type.
 *  @return Void.
 */
void idt_add_desc(int idt_entry, void *handler, unsigned gate_type, unsigned dpl)
{
    idt_desc_t *idt_desc = (idt_desc_t *)((uint64_t *)idt_base() + idt_entry);
    idt_desc->offset_l = (uint16_t)(int)(handler);
    idt_desc->segment_selector = SEGSEL_KERNEL_CS;
    idt_desc->zeros = 0x00;
    idt_desc->flags = (gate_type | IDT_D | dpl | IDT_P);
    idt_desc->offset_h = (uint16_t)(((int)handler) >> 16);
}

/** @brief Initializes the IDT entries for each interrupt that must be handled.
 *
 *  @return Void.
 */
void idt_init() {

    /* Add timer and keyboard interrupt gate descriptors */
    if (timer_init() == 0) {
        idt_add_desc(TIMER_IDT_ENTRY, timer_handler_int, IDT_INT, IDT_DPL_KERNEL);
    }
    if (keyboard_init() == 0) {
        idt_add_desc(KEY_IDT_ENTRY, keyboard_int, IDT_INT, IDT_DPL_KERNEL);
    }

    /* Add system call trap gate descriptors */
    idt_add_desc(FORK_INT, fork_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(EXEC_INT, exec_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(WAIT_INT, wait_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(YIELD_INT, yield_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(DESCHEDULE_INT, deschedule_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(MAKE_RUNNABLE_INT, make_runnable_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(GETTID_INT, gettid_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(NEW_PAGES_INT, new_pages_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(REMOVE_PAGES_INT, remove_pages_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(SLEEP_INT, sleep_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(READLINE_INT, readline_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(PRINT_INT, print_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(SET_TERM_COLOR_INT, set_term_color_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(SET_CURSOR_POS_INT, set_cursor_pos_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(GET_CURSOR_POS_INT, get_cursor_pos_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(THREAD_FORK_INT, thread_fork_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(GET_TICKS_INT, get_ticks_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(MISBEHAVE_INT, misbehave_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(HALT_INT, halt_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(SET_STATUS_INT, set_status_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(VANISH_INT, vanish_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(READFILE_INT, readfile_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(SWEXN_INT, swexn_int, IDT_TRAP, IDT_DPL_USER);

    idt_add_desc(IDT_DE, exn_divide_wrapper, IDT_INT, IDT_DPL_USER);
    idt_add_desc(IDT_DB, exn_debug_wrapper, IDT_INT, IDT_DPL_USER);
    idt_add_desc(IDT_BP, exn_breakpoint_wrapper, IDT_INT, IDT_DPL_USER);
    idt_add_desc(IDT_OF, exn_overflow_wrapper, IDT_INT, IDT_DPL_USER);
    idt_add_desc(IDT_BR, exn_boundcheck_wrapper, IDT_INT, IDT_DPL_USER);
    idt_add_desc(IDT_UD, exn_opcode_wrapper, IDT_INT, IDT_DPL_USER);
    idt_add_desc(IDT_NM, exn_nofpu_wrapper, IDT_INT, IDT_DPL_USER);
    idt_add_desc(IDT_NP, exn_segfault_wrapper, IDT_INT, IDT_DPL_USER);
    idt_add_desc(IDT_SS, exn_stackfault_wrapper, IDT_INT, IDT_DPL_USER);
    idt_add_desc(IDT_GP, exn_protfault_wrapper, IDT_INT, IDT_DPL_USER);
    idt_add_desc(IDT_PF, exn_pagefault_wrapper, IDT_INT, IDT_DPL_USER);
    idt_add_desc(IDT_MF, exn_fpufault_wrapper, IDT_INT, IDT_DPL_USER);
    idt_add_desc(IDT_AC, exn_alignfault_wrapper, IDT_INT, IDT_DPL_USER);
    idt_add_desc(IDT_XF, exn_simdfault_wrapper, IDT_INT, IDT_DPL_USER);
    // TODO add mising exceptions...
}
