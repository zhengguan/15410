/** @file ide-config.h
 *  @brief Interface between Pebbles kernel and the 410 IDE driver.
 *
 *  This is the only file in the IDE driver which you are permitted to
 *  modify. It contains a set of macros which allow the driver to access
 *  the kernel internals which it needs.
 *
 *  @author Joshua Wise (jwise) <joshua@joshuawise.com>
 */

#ifndef __IDE_CONFIG_H
#define __IDE_CONFIG_H

/* XXX: In addition to changing the contents of this file, remember to
 * ensure that ide_interrupt_handler() is called whenever interrupt
 * IDE_INTERRUPT occurs.
 */

/* XXX: #include any other needed files.
 */

/* XXX: You may wish to put your own debug macro here.
 */
#define _IDE_DEBUG(...)		lprintf(__VA_ARGS__)

#endif /* __IDE_CONFIG_H */
