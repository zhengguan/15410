/** @file fork.c
 *  @brief Implements fork
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
#include <fork.h>
#include <scheduler.h>
#include <proc.h>
#include <simics.h>
#include <proc.h>
#include <syscall.h>
#include <context_switch_asm.h>
#include <asm.h>
#include <cr.h>
#include <vm.h>
#include <string.h>

int fork()
{
    tcb_t *old_tcb;
    hashtable_get(&tcbs, gettid(), (void**)&old_tcb);

    disable_interrupts();

    int tid = 0;

    if ((tid = proc_new_process()) < 0) {
        lprintf("fucked up7");
        MAGIC_BREAK;
    }
        
    if (store_regs(&old_tcb->regs)) {
        
        unsigned old_esp0 = get_esp0();
        lprintf("old_esp0: %u\n", old_esp0);
        
        // Hoepfully this is the new value
        unsigned new_esp0 = get_esp0();
        lprintf("new_esp0: %u\n", new_esp0);

        int i;
        for (i = 0; i < KERNEL_STACK_SIZE; i++)
            ((char *)(new_esp0 - KERNEL_STACK_SIZE))[i] = ((char *)(old_esp0 - KERNEL_STACK_SIZE))[i];
        // memcpy((void *)(new_esp0 - KERNEL_STACK_SIZE), (void *)(old_esp0 - KERNEL_STACK_SIZE), KERNEL_STACK_SIZE);

        lprintf("1: %d", tid);

        //may be bad?
        unsigned cr3 = vm_copy();
        lprintf("1.5: %d", tid);
        set_cr3(cr3);

        lprintf("2: %d", tid);

        enable_interrupts();
        return 0;

    } else {
        enable_interrupts();
        MAGIC_BREAK;
        lprintf("3: %d", tid);
        
        return tid;
    }
}
