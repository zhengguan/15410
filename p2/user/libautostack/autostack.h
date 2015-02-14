/** @file autostack.c
 *  @brief Prototypes and macros for the exception handler responsible for
 *  performing automatic stack growth.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _AUTOSTACK_H_
#define _AUTOSTACK_H_

#define PAGE_MASK ((unsigned int)(~(PAGE_SIZE - 1)))

typedef struct stackinfo {
    void *stack_high;
    void *stack_low;
    unsigned int stack_max_size;
} stackinfo_t;

extern stackinfo_t g_stackinfo;

#endif /* _AUTOSTACK_H_ */
