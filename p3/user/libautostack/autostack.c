/** @file autostack.c
 *  @brief Registers a exception handler which resolveds page-fault
 *  exceptions by performing automatic stack growth.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <autostack.h>
#include <stdlib.h>
#include <syscall.h>
#include <ureg.h>

#define MAIN_EXCEPTION_STACK_SIZE PAGE_SIZE

stackinfo_t g_stackinfo;
void *main_exception_stack;

// static void register_exception_handler(void *esp3, ureg_t *newureg);
// static void exception_handler(void *arg, ureg_t *ureg);

void install_autostack(void *stack_high, void *stack_low)
{
    g_stackinfo.stack_high = stack_high;
    g_stackinfo.stack_low = stack_low;

    if ( (main_exception_stack = malloc(MAIN_EXCEPTION_STACK_SIZE)) == NULL ) {
        exit(-1);
    }

    // register_exception_handler(main_exception_stack, NULL);
}

/** @brief Registers the exception handler responsible for  performing
 *  automatic stack growth.
 *
 *  @param esp3 Exception stack pointer.
 *  @param newureg New register values.
 *  @return Void.
 */
// static void register_exception_handler(void *esp3, ureg_t *newureg) {
//     swexn(esp3, exception_handler, NULL, newureg);
// }

/** @brief Handles page-faults by performing automatic stack growth.
 *
 *  @param arg Argument pointer.
 *  @param ureg Register values.
 *  @return Void.
 */
// static void exception_handler(void *arg, ureg_t *ureg) {
//     if ((ureg == NULL) || (ureg->cause != SWEXN_CAUSE_PAGEFAULT)) {
//         exit(-1);
//     }

//     void *fault_addr = (void *)ureg->cr2;

//     void *base = (void *)((unsigned int)fault_addr & PAGE_MASK);
//     int len = (int)(g_stackinfo.stack_low - base);

//     if (new_pages(base, len) < 0) {
//         exit(-2);
//     }

//     g_stackinfo.stack_low = base;

//     register_exception_handler(main_exception_stack, ureg);
// }
