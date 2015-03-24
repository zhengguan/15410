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
// #include <syscall.h>
#include <vm.h>
#include <common_kern.h>
#include <stdlib.h>
#include <asm_exception.h>
#include <asm.h>
#include <hashtable.h>
#include <idt.h>
#include <asm_common.h>
#include <proc.h>

#define HASHTABLE_SIZE 100

typedef void (*swexn_handler_wrapper_t)(void *arg, ureg_t *ureg, swexn_handler_t);

swexn_handler_wrapper_t wrapper;

typedef struct {
    swexn_handler_wrapper_t wrapper;
    swexn_handler_t eip;
    void *esp3;
    void *arg;
} reg_handler_t;

hashtable_t handler_ht;
hashtable_t ureg_ht;

bool ht_inited = false;

static int call_user_handler(ureg_t *exn_ureg)
{
    if (!ht_inited)
        return -1;

    disable_interrupts();

    reg_handler_t *rhp;
    if (hashtable_get(&handler_ht, getpid(), (void**)&rhp) < 0) {
        return -1;
    }

    hashtable_remove(&handler_ht, getpid());

    typedef struct {
        unsigned ret;
        swexn_handler_t eip;
        void *arg;
        ureg_t ureg;
    } handler_args_t;

    char *esp = (char*)rhp->esp3 - sizeof(handler_args_t);

    if (!vm_is_present_len(esp, sizeof(handler_args_t))) {
        return -2;
    }

    //save ureg
    ureg_t *ureg = malloc(sizeof(ureg_t));
    *ureg = *exn_ureg;
    hashtable_add(&ureg_ht, getpid(), (void*)ureg);

    //setup args
    ((handler_args_t *)esp)->eip = rhp->eip;
    ((handler_args_t *)esp)->arg = rhp->arg;
    ((handler_args_t *)esp)->ureg = *exn_ureg;

    //where to switch to
    ureg_t handler_ureg = *exn_ureg;
    handler_ureg.esp = (unsigned)esp;
    handler_ureg.eip = (unsigned)rhp->wrapper;
    free(rhp);

    lprintf("%p", exn_ureg);

    lprintf("calling user handler");

    jmp_ureg(&handler_ureg); //reenables interrupts

    return -3;
}

void exception_handler(ureg_t ureg)
{
    lprintf("exception %u", ureg.cause);
    ureg.zero = 0;
    if ((ureg.cs & 3) == 0) {
        lprintf("kernel mode error");
        MAGIC_BREAK;
    } else {
        switch (ureg.cause) {
            case IDT_NMI: /* Non-Maskable Interrupt (Interrupt) */
            case IDT_DF:  /* Double Fault (Abort) */
            case IDT_CSO: /* Coprocessor Segment Overrun (Fault) */
            case IDT_TS:  /* Invalid Task Segment Selector (Fault) */
            case IDT_MC:  /* Machine Check (Abort) */
                MAGIC_BREAK;
                break;

            /*exceptions passed to user */
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
                int err = call_user_handler(&ureg); //if comes back, we failed
                lprintf("handler not correctly registered in user thread: %d. Killing.", err);
                MAGIC_BREAK;
                //KILL THREAD
                enable_interrupts();
            }
        }
    }
}


int kern_swexn(swexn_handler_wrapper_t wrapper, void *esp3, swexn_handler_t eip, void *arg, ureg_t *newureg)
{
    if ((esp3 && (unsigned)esp3 < USER_MEM_START) || (eip && (unsigned)eip < USER_MEM_START) ||  (unsigned)wrapper < USER_MEM_START) {
        return -1;
    }

    disable_interrupts();
    if (!ht_inited) {
        if (hashtable_init(&handler_ht, HASHTABLE_SIZE) < 0 ||
            hashtable_init(&ureg_ht, HASHTABLE_SIZE) < 0)
            return -3;

        ht_inited = true;
    }
    enable_interrupts();

    if (esp3 == NULL || eip == NULL) {
        hashtable_remove(&handler_ht, getpid());
    } else {
        reg_handler_t *rhp;
        disable_interrupts();
        if (hashtable_get(&handler_ht, getpid(), (void**)&rhp) < 0) {
            rhp = malloc(sizeof(reg_handler_t));
            if (rhp == NULL)
                return -2;
            hashtable_add(&handler_ht, getpid(), rhp);
        }

        rhp->esp3 = esp3;
        rhp->eip = eip;
        rhp->arg = arg;
        rhp->wrapper = wrapper;
        enable_interrupts();
    }

    if (newureg == NULL)
        return 0;

    disable_interrupts();

    if (!vm_is_present_len(newureg, sizeof(ureg_t))) {
        enable_interrupts();
        return -3;
    }

    //FIXME: very unsafe. can set registers to anything.
    jmp_ureg(newureg); //reenables interrupts before jmp

    return -2;
}


int exn_handler_complete()
{
    if (!ht_inited)
        return -1;
    ureg_t *ureg;
    disable_interrupts();
    if(hashtable_get(&ureg_ht, getpid(), (void**)&ureg) < 0)
        return -1;
    hashtable_remove(&ureg_ht, getpid());
    ureg_t stack_ureg = *ureg;
    free(ureg);
    jmp_ureg(&stack_ureg);
    //shouldnt get here
    return -2;
}