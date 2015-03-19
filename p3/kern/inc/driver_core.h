/** @file driver_core.h
 *  @brief An interface for core driver functions
 *
 *  This contains the interface for the core driver functions to be used
 *  by multiple driver implementations
 *
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef __DRIVER_CORE_H
#define __DRIVER_CORE_H

void notify_interrupt_complete();
int handler_install(void (*tickback)(unsigned));

#endif /* __DRIVER_CORE_H */
