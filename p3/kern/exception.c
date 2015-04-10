/** @file exception.c
 *
 *  This file implements the handling of exceptions in
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <exception.h>
#include <simics.h>
#include <vm.h>
#include <common_kern.h>
#include <stdlib.h>
#include <exception_asm.h>
#include <asm.h>
#include <hashtable.h>
#include <idt.h>
#include <asm_common.h>
#include <proc.h>

typedef struct {
    unsigned ret;
    void *arg;
    void *ureg_ptr;
    ureg_t ureg;
} handler_args_t;

handler_t *get_swexn_handler(pcb_t *pcb)
{
    return &(pcb->swexn_handler);
}

static int call_user_handler(ureg_t *ureg)
{
    disable_interrupts();

    handler_t *handler = get_swexn_handler(*CUR_PCB);

    if (handler->eip == NULL)
        return -2;

    unsigned esp = (unsigned)handler->esp3 - sizeof(handler_args_t);
    if (!vm_is_present_len((void*)esp, sizeof(handler_args_t))) {
        return -3;
    }

    // setup args
    ((handler_args_t *)esp)->arg = handler->arg;
    ((handler_args_t *)esp)->ureg_ptr = &(((handler_args_t *)esp)->ureg);
    ((handler_args_t *)esp)->ureg = *ureg;
    unsigned eip = (unsigned)handler->eip;

    deregister_swexn_handler(*CUR_PCB);

    jmp_user(eip, esp); // reenables interrupts

    panic("returned from user handler");
    return -4;
}

void exception_handler(ureg_t ureg)
{
    lprintf("exception %d in %d", ureg.cause, getpid());
    if ((ureg.cs & 3) == 0) {
        lprintf("kernel mode exception %u", ureg.cause);
        MAGIC_BREAK;
    }

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
        case IDT_PF: {  /* Page Fault (Fault) */
            call_user_handler(&ureg);
        }
    }

    enable_interrupts();
    proc_kill_thread("Exception %d", ureg.cause);
}

int swexn(void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg)
{
    //check return ureg
    if (newureg && (!vm_is_present_len(newureg, sizeof(ureg_t))
                    || (unsigned)newureg < USER_MEM_START))
        return -2;

    //if one null, remove handler
    if (esp3 == NULL || eip == NULL) {
        deregister_swexn_handler(*CUR_PCB);
    } else {
        if ((unsigned)eip < USER_MEM_START || !vm_is_present(eip))
            return -1;
        if ((unsigned)esp3 < USER_MEM_START ||
            !vm_is_present_len((void*)((unsigned)esp3-sizeof(handler_args_t)),
                                                      sizeof(handler_args_t))) {
            return -2;
        }

        register_swexn_handler(*CUR_PCB, eip, esp3, arg);
    }

    if (newureg == NULL) {
        return 0;
    }

    jmp_ureg_user(newureg); //reenables interrupts before jmp

    return -5;
}


void deregister_swexn_handler(pcb_t *pcb)
{
    get_swexn_handler(pcb)->eip = NULL;
}

int register_swexn_handler(pcb_t *pcb, swexn_handler_t eip, void *esp3, void *arg)
{
    handler_t *handler = get_swexn_handler(pcb);
    handler->esp3 = esp3;
    handler->eip = eip;
    handler->arg = arg;
    return 0;
}

int dup_swexn_handler(pcb_t *src_pcb, pcb_t *dest_pcb)
{
    handler_t *src_handler = get_swexn_handler(src_pcb);
    return register_swexn_handler(dest_pcb, src_handler->eip, src_handler->esp3, src_handler->arg);
}