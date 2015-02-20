/** @file atom_xchg.h
 *  @brief This file defines the function prototype for atom_xchg.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _ATOM_XCHG_H
#define _ATOM_XCHG_H
 
/** @brief Performs an atomic xchg.
 *
 *  @param dest Destination.
 *  @param src Source
 *  @return The old dest value.
 */
extern int atom_xchg(int *dest, int src);

#endif /* _ATOM_XCHG_H */
