/** @file driver_init.c
 *  @brief Implementation of the functions needed to initialize drivers.
 *
 *  This contains the functions definitions for the functions needed to
 *  initialize the timer and keyboard drivers.
 *  This contains functions to install the IDT entries for the timer and
 *  keyboard interrupt handlers.
 *
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include "driver_core.h"
#include "int_handler_wrapper.h"
#include "keyboard_driver.h"
#include "timer_driver.h"

#include <x86/asm.h>
#include <x86/interrupt_defines.h>
#include <x86/keyhelp.h>
#include <x86/seg.h>
#include <x86/timer_defines.h>

#include <stdint.h>

//defines the privilige level, present bit, and size of the trap gate
#define IDT_ATTR 0x8F

//a struct representing the layout of the trap gate in memory
typedef struct {
	uint16_t lower_offset;  //less significant offset bits
	uint16_t selector;
	uint8_t zero; 					//always 0
	uint8_t attributes;
	uint16_t higher_offset; //more significant offset bits
} idt_entry;

/**
 * @brief Installs an entry into the interrupt descriptor table
 * @param idx The index into the table
 * @param handler A pointer to the handler function.
 * This should be a void (*) (void) function.
 * @param selector The selector of the trap gate
 * @param attributes The attributes of the trap gate including the
 * privilige level, present bit, and size of the trap gate
 */
void install_idt_entry(unsigned idx, void (*handler)(void),
											 uint16_t selector, uint8_t attributes)
{
	idt_entry *base = idt_base();

	//create the idt entry struct
	idt_entry entry = {0};
	entry.lower_offset = ((uint32_t)handler) & 0xFFFF;
	entry.higher_offset = (((uint32_t)handler) >> 16) & 0xFFFF;
	entry.selector = selector;
	entry.attributes = attributes;

	*(base + idx) = entry;
}

/**
 * @brief Installs driver interrupt into the IDT
 * @param tickback The function to call on a timer tick. Takes as an argument
 * the total number of timer interrupts received since the kernel began running.
 * @return 0 on success
 * -1 on timer install failure
 * -2 on keyboard install failure
 */
int handler_install(void (*tickback)(unsigned))
{
	//save the tickback to the timer driver
	tick_callback = tickback;

	//setup the timer
	if (timer_setup())
		install_idt_entry(TIMER_IDT_ENTRY, timer_handler_wrapper,
											SEGSEL_KERNEL_CS, IDT_ATTR);
	else
		return -1;

	//setup the keyboard
	if (keyboard_setup())
		install_idt_entry(KEY_IDT_ENTRY, keyboard_handler_wrapper,
											SEGSEL_KERNEL_CS, IDT_ATTR);
	else
		return -2;

	//success
  return 0;
}
