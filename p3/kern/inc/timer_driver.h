/** @file timer_driver.h
 *  @brief An interface for the timer driver library
 *
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
#ifndef __TIMER_DRIVER_H
#define __TIMER_DRIVER_H

extern void (*tick_callback)(unsigned);

int timer_setup();

#endif /* __TIMER_DRIVER_H */