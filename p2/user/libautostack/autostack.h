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

#include <ureg.h>

#define PAGE_MASK ((unsigned int)(~(PAGE_SIZE - 1)))
#define ROOT_HANDLER_STACK (void *)0x90000000

typedef struct stackinfo {
    void *stack_high;
    void *stack_low;
    unsigned int stack_max_size;
} stackinfo_t;

void register_exception_handler(void *esp3, ureg_t *newureg);

void exception_handler(void *arg, ureg_t *ureg);

#endif /* _AUTOSTACK_H_ */
