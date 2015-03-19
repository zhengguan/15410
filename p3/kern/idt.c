/** @file idt.c
 *  @brief Initializes the IDT entries.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <idt.h>
#include <x86/asm.h>
#include <x86/seg.h>
#include <syscall_int.h>
#include <handler.h>
#include <asm_noop.h>
#include <scheduler.h>

extern int new_pages(void *base, int len);
extern int remove_pages(void *base);

void noop()
{
    return;
}

/** @brief Initializes the IDT entries for each interrupt that must be handled.
 *
 *  @return Void.
 */
void idt_init() {

    /* Add timer and keyboard interrupt gate descriptors */
    idt_add_desc(TIMER_IDT_ENTRY, scheduler_tick, IDT_INT, IDT_DPL_KERNEL);
    idt_add_desc(KEY_IDT_ENTRY, asm_noop, IDT_INT, IDT_DPL_KERNEL);

    /* Add system call trap gate descriptors */
    // idt_add_desc(SYSCALL_INT, 0, IDT_TRAP, IDT_DPL_USER);
    // idt_add_desc(FORK_INT, 0, IDT_TRAP, IDT_DPL_USER);
    // idt_add_desc(EXEC_INT, 0, IDT_TRAP, IDT_DPL_USER);
    // idt_add_desc(WAIT_INT, 0, IDT_TRAP, IDT_DPL_USER);
    // idt_add_desc(YIELD_INT, 0, IDT_TRAP, IDT_DPL_USER);
    // idt_add_desc(DESCHEDULE_INT, 0, IDT_TRAP, IDT_DPL_USER);
    // idt_add_desc(MAKE_RUNNABLE_INT, 0, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(GETTID_INT, gettid_int, IDT_TRAP, IDT_DPL_USER);
    idt_add_desc(NEW_PAGES_INT, new_pages_int, IDT_TRAP, IDT_DPL_USER);
    //idt_add_desc(REMOVE_PAGES_INT, remove_pages, IDT_TRAP, IDT_DPL_USER);
    // idt_add_desc(SLEEP_INT, 0, IDT_TRAP, IDT_DPL_USER);
    // idt_add_desc(GETCHAR_INT, 0, IDT_TRAP, IDT_DPL_USER);
    // idt_add_desc(READLINE_INT, 0, IDT_TRAP, IDT_DPL_USER);
    // idt_add_desc(PRINT_INT, 0, IDT_TRAP, IDT_DPL_USER);
    // idt_add_desc(SET_TERM_COLOR_INT, 0, IDT_TRAP, IDT_DPL_USER);
    // idt_add_desc(SET_CURSOR_POS_INT, 0, IDT_TRAP, IDT_DPL_USER);
    // idt_add_desc(GET_CURSOR_POS_INT, 0, IDT_TRAP, IDT_DPL_USER);
    // idt_add_desc(THREAD_FORK_INT, 0, IDT_TRAP, IDT_DPL_USER);
    // idt_add_desc(GET_TICKS_INT, 0, IDT_TRAP, IDT_DPL_USER);
    // idt_add_desc(MISBEHAVE_INT, 0, IDT_TRAP, IDT_DPL_USER);
    // idt_add_desc(HALT_INT, 0, IDT_TRAP, IDT_DPL_USER);
    // idt_add_desc(TASK_VANISH_INT, 0, IDT_TRAP, IDT_DPL_USER);
    // idt_add_desc(SET_STATUS_INT, 0, IDT_TRAP, IDT_DPL_USER);
    // idt_add_desc(VANISH_INT, 0, IDT_TRAP, IDT_DPL_USER);
    // idt_add_desc(READFILE_INT, 0, IDT_TRAP, IDT_DPL_USER);
    // idt_add_desc(SWEXN_INT, 0, IDT_TRAP, IDT_DPL_USER);
}

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
