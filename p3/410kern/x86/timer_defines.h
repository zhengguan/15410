/**
 * @file x86/timer_defines.h
 * Date: January 15th 2003
 * Author: Steve Muckle <smuckle@andrew.cmu.edu>
 *
 * Important defines for the timer.
 *
 * @bug This is, at the moment, a quite 410 specific slice of the PIT's
 *      capabilities.  If somebody wants to, for example, drive the PC
 *      speaker for real, it would be great if they fleshed this out a bit
 *      and submitted patches.
 */
#ifndef _TIME_DEFINES_H_
#define _TIME_DEFINES_H_

#define TIMER_RATE 1193182

#define TIMER_IDT_ENTRY 0x20
#define TIMER_PERIOD_IO_PORT 0x40
#define TIMER_MODE_IO_PORT 0x43
#define TIMER_SQUARE_WAVE 0x36
#define TIMER_ONE_SHOT 0x30

#endif
