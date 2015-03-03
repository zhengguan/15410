/** @file cr3.h
 *  @brief This file defines the function prototype for set_cr3.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _CR3_H
#define _CR3_H
 
/** @brief Sets the %cr3 control register.
 *
 *  @param dest The new %cr3 value.
 *  @return Void.
 */
extern void set_cr3(void *cr3);

/** @brief Gets the %cr3 control register.
 *
 *  @return The %cr3 value.
 */
extern void *get_cr3();

#endif /* _CR3_H */
