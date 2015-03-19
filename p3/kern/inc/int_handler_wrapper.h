/** @file int_handler_wrapper.h
 *  @brief Function prototypes for the interrupt handler wrapper functions
 *
 *  This contains prototypes for interrupt handler wrappers.
 *  These functions are necessary in order to save and restore the cpu
 *  registers that are normally caller saved however cannot be
 *  when an interrupt is received.
 *
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef __INT_HANDLER_WRAP_H
#define __INT_HANDLER_WRAP_H

/**
 * @brief Wrapper for the timer handler function
 * @details Calls timer_handler(), saving registers beforehand
 * and restoring afterward.
 */
void timer_handler_wrapper();

/**
 * @brief Wrapper for the keyboard handler function
 * @details Calls keyboard_handler(), saving registers beforehand
 * and restoring afterward.
 */
void keyboard_handler_wrapper();

#endif /* __INT_HANDLER_WRAP_H */