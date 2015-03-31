/** @file exception.c
 *
 *  This file implements the handling of exceptions in
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <simics.h>
#include <ureg.h>
#include <vm.h>
#include <common_kern.h>
#include <stdlib.h>
#include <exception_asm.h>
#include <asm.h>
#include <hashtable.h>
#include <idt.h>
#include <asm_common.h>
#include <proc.h>

#define HASHTABLE_SIZE 100

typedef void (*swexn_handler_wrapper_t)(void *arg, ureg_t *ureg, swexn_handler_t);

swexn_handler_wrapper_t wrapper;

typedef struct {
    unsigned ret;
    swexn_handler_t eip;
    void *arg;
    ureg_t ureg;
} handler_args_t;
    
typedef struct {
    swexn_handler_wrapper_t wrapper;
    swexn_handler_t eip;
    void *esp3;
    void *arg;
} handler_t;

hashtable_t handler_ht;
hashtable_t ureg_ht;

bool ht_inited = false;

static int call_user_handler(ureg_t *ureg)
{
    if (!ht_inited)
        return -1;

    disable_interrupts();

    handler_t *handler;
    if (hashtable_get(&handler_ht, getpid(), (void**)&handler) < 0) {
        return -2;
    }

    hashtable_remove(&handler_ht, getpid());

    char *esp = (char*)handler->esp3 - sizeof(handler_args_t);
    if (!vm_is_present_len(esp, sizeof(handler_args_t))) {
        return -3;
    }

    // save ureg
    ureg_t *copy_ureg = malloc(sizeof(ureg_t));
    *copy_ureg = *ureg;
    hashtable_add(&ureg_ht, getpid(), (void*)copy_ureg);

    // setup args
    ((handler_args_t *)esp)->eip = handler->eip;
    ((handler_args_t *)esp)->arg = handler->arg;
    ((handler_args_t *)esp)->ureg = *ureg;

    //where to switch to
    ureg_t handler_ureg = *ureg;
    handler_ureg.esp = (unsigned)esp;
    handler_ureg.eip = (unsigned)handler->wrapper;
    free(handler);

    lprintf("%p", ureg);

    lprintf("calling user handler");

    jmp_ureg(&handler_ureg); // reenables interrupts

    // shouldn't get here
    return -3;
}

void exception_handler(ureg_t ureg)
{
    lprintf("exception %u", ureg.cause);
    
    // TODO why is this here?
    if ((ureg.cs & 3) == 0) {
        lprintf("kernel mode error");
        MAGIC_BREAK;
    }
        
    ureg.zero = 0;
    
    switch (ureg.cause) {
        case IDT_NMI: /* Non-Maskable Interrupt (Interrupt) */
        case IDT_DF:  /* Double Fault (Abort) */
        case IDT_CSO: /* Coprocessor Segment Overrun (Fault) */
        case IDT_TS:  /* Invalid Task Segment Selector (Fault) */
        case IDT_MC:  /* Machine Check (Abort) */
            MAGIC_BREAK;
            break;

        /* exceptions passed to user */
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
            int err = call_user_handler(&ureg); // if comes back, we failed
            lprintf("handler not correctly registered in user thread: %d. Killing.", err);
            MAGIC_BREAK;
            //KILL THREAD
            enable_interrupts();
        }
    }
}

int kern_swexn(swexn_handler_wrapper_t wrapper, void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg)
{
    if ((esp3 && (unsigned)esp3 < USER_MEM_START) || (eip && (unsigned)eip < USER_MEM_START) || (unsigned)wrapper < USER_MEM_START) {
        return -1;
    }

    // TODO is disable interrupts necessary here?
    // TODO move to init function?
    disable_interrupts();
    if (!ht_inited) {
        if (hashtable_init(&handler_ht, HASHTABLE_SIZE) < 0 ||
            hashtable_init(&ureg_ht, HASHTABLE_SIZE) < 0)
            return -2;

        ht_inited = true;
    }
    enable_interrupts();

    if (esp3 == NULL || eip == NULL) {
        hashtable_remove(&handler_ht, getpid());
    } else {
        handler_t *handler;
        disable_interrupts();
        if (hashtable_get(&handler_ht, getpid(), (void **)&handler) < 0) {
            handler = malloc(sizeof(handler_t));
            if (handler == NULL) {
                return -3;
            }
            hashtable_add(&handler_ht, getpid(), (void *)handler);
        }

        handler->esp3 = esp3;
        handler->eip = eip;
        handler->arg = arg;
        handler->wrapper = wrapper;
        enable_interrupts();
    }

    if (newureg == NULL) {
        return 0;
    }

    disable_interrupts();

    if (!vm_is_present_len(newureg, sizeof(ureg_t))) {
        enable_interrupts();
        return -4;
    }

    //FIXME: very unsafe. can set registers to anything.
    jmp_ureg(newureg); //reenables interrupts before jmp

    return -5;
}

int exn_handler_complete()
{
    if (!ht_inited) {
        return -1;
    }
    
    ureg_t *ureg;
    
    disable_interrupts();
    
    if(hashtable_get(&ureg_ht, getpid(), (void**)&ureg) < 0) {
        return -2;
    }
    
    hashtable_remove(&ureg_ht, getpid());
    
    ureg_t tmp_ureg = *ureg;
    free(ureg);
    
    jmp_ureg(&tmp_ureg);
    
    // shouldn't get here
    return -2;
}
