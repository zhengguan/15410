/** @file exception.c
 *
 *  This file implements the handling of exceptions in
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <exception.h>
#include <vm.h>
#include <common_kern.h>
#include <stdlib.h>
#include <exception_asm.h>
#include <asm.h>
#include <hashtable.h>
#include <idt.h>
#include <asm_common.h>
#include <proc.h>
#include <vm.h>
#include <kern_common.h>
#include <seg.h>
#include <assert.h>
#include <asm_common.h>

typedef struct {
    unsigned ret;
    void *arg;
    void *ureg_ptr;
    ureg_t ureg;
} handler_args_t;


/**
 * @brief Gets the registered software exception handler for a process.
 *
 * @param tcb The tcb of the process to get the handler for.
 * @return The handler
 */
static handler_t *get_swexn_handler(tcb_t *tcb)
{
    return &(tcb->swexn_handler);
}

/**
 * @brief Deregisters a registered software exception handler for a process
 * if one exists.
 * @details Has no effect if no handler is registered.
 *
 * @param tcb The tcb of the process to deregister the handler for.
 */
void deregister_swexn_handler(tcb_t *tcb)
{
    get_swexn_handler(tcb)->eip = NULL;
}

/**
 * @brief Registers a software exception handler for a process.
 * @details Replaces any existing handler.
 *
 * @param tcb The tcb of the process to register the handler for.
 * @param eip The handler function.
 * @param esp3 The bottom of the handler stack.
 * @param arg The argument to pass the handler function.
 */
void register_swexn_handler(tcb_t *tcb, swexn_handler_t eip, void *esp3, void *arg)
{
    handler_t *handler = get_swexn_handler(tcb);
    handler->esp3 = esp3;
    handler->eip = eip;
    handler->arg = arg;
}

/**
 * @brief Duplicates a software exception handler from one process to another.
 * @param src_tcb The tcb of the source process.
 * @param dest_tcb The tcb of the destination process.
 */
void dup_swexn_handler(tcb_t *src_tcb, tcb_t *dest_tcb)
{
    handler_t *src_handler = get_swexn_handler(src_tcb);
    register_swexn_handler(dest_tcb, src_handler->eip, src_handler->esp3,
                                                       src_handler->arg);
}

/**
 * @brief Calls a registered user exception handler.
 * @param ureg The ureg to pass to the user handler.
 * @return Doesn't return on success. A negative error code on error.
 */
static int call_user_handler(ureg_t *ureg)
{

    tcb_t *tcb = gettcb();
    handler_t *handler = get_swexn_handler(tcb);

    if (handler->eip == NULL) {
        return -2;
    }

    unsigned esp = (unsigned)handler->esp3 - sizeof(handler_args_t);

    if (buf_lock_rw(sizeof(handler_args_t), (char*)esp) < 0) {
        return -3;
    }

    // setup args
    ((handler_args_t *)esp)->arg = handler->arg;
    ((handler_args_t *)esp)->ureg_ptr = &(((handler_args_t *)esp)->ureg);
    ((handler_args_t *)esp)->ureg = *ureg;

    buf_unlock(sizeof(handler_args_t), (char*)esp);

    unsigned eip = (unsigned)handler->eip;

    deregister_swexn_handler(tcb);

    jmp_user(eip, esp);

    panic("Returned from swexn handler");
    return -4;
}

/**
 * @brief Handles all x86 exceptions.
 * @details If a user exception handler is registered, this will be called
 * before killing a thread.
 * @param ureg The ureg containing the registers at the time of the exception.
 */
void exception_handler(ureg_t ureg)
{
    if ((ureg.cs & 3) == 0) {
        panic("Kernel mode exception %u", ureg.cause);
    }

    enable_interrupts();

    ureg.zero = 0;

    switch (ureg.cause) {
        case IDT_NMI: /* Non-Maskable Interrupt (Interrupt) */
        case IDT_DF:  /* Double Fault (Abort) */
        case IDT_CSO: /* Coprocessor Segment Overrun (Fault) */
        case IDT_TS:  /* Invalid Task Segment Selector (Fault) */
        case IDT_MC:  /* Machine Check (Abort) */
            break;

        // Exceptions passed to user
        case IDT_DE:  /* Devision Error (Fault) */
        case IDT_DB:  /* Debug Exception (Fault/Trap) */
        case IDT_BP:  /* Breakpoint (Trap) */
        case IDT_OF:  /* Overflow (Trap) */
        case IDT_BR:  /* BOUND Range exceeded (Fault) */
        case IDT_UD:  /* UnDefined Opcode (Fault) */
        case IDT_NM:  /* No Math coprocessor (Fault) */
        case IDT_NP:  /* Segment Not Present (Fault) */
        case IDT_SS:  /* Stack Segment Fault (Fault) */
        case IDT_GP:  /* General Protection Fault (Fault) */
        case IDT_MF:  /* X87 Math Fault (Fault) */
        case IDT_AC:  /* Alignment Check (Fault) */
        case IDT_XF:  /* SSE Floating Point Exception (Fault) */
            ureg.cr2 = 0;
        case IDT_PF:  /* Page Fault (Fault) */
            call_user_handler(&ureg);
    }

    proc_kill_thread("Exception %d", ureg.cause);
}

int swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg)
{
    ureg_t newureg_kern;
    if (newureg) {
        if (buf_lock(sizeof(ureg_t), (char*)newureg) < 0)
            return -3;

        if (newureg->cs != SEGSEL_USER_CS ||
            newureg->ss != SEGSEL_USER_DS ||
            control_eflags(newureg->eflags) != USER_EFLAGS)
            return -4;
        newureg_kern = *newureg;
        buf_unlock(sizeof(ureg_t), (char*)newureg);
    }

    tcb_t *tcb = gettcb();

    //if one null, remove handler
    if (esp3 == NULL || eip == NULL) {
        deregister_swexn_handler(tcb);
    } else {
        //Check vm present for user's sake. No locks needed.
        if ((unsigned)eip < USER_MEM_START || !vm_check_flags(getpcb()->pd, eip, USER_FLAGS_RO, 0)) {
            return -1;
        }
        if ((unsigned)esp3 < USER_MEM_START ||
            !vm_check_flags_len(getpcb()->pd, (void*)((unsigned)esp3-sizeof(handler_args_t)),
                                                      sizeof(handler_args_t), USER_FLAGS_RW, 0)) {
            return -2;
        }

        register_swexn_handler(tcb, eip, esp3, arg);
    }

    if (newureg) {
        jmp_ureg_user(&newureg_kern);
    }

    return 0;
}