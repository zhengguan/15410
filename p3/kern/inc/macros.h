/** @file macros.h
 *  @brief Prototypes for commonly used macros.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _MACROS_H
#define _MACROS_H

#include <syscall.h>

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
#define MAX(X,Y) ((X) > (Y) ? (X) : (Y))

#define PAGE_MASK (~(PAGE_SIZE - 1))
#define ROUND_DOWN_PAGE(ADDR) ((unsigned)(ADDR) & PAGE_MASK)
#define ROUND_UP_PAGE(ADDR) (ROUND_DOWN_PAGE((ADDR) + PAGE_SIZE - 1))

#endif /* _MACROS_H */
