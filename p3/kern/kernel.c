/** @file kernel.c
 *  @brief An initial kernel.c
 *
 *  You should initialize things in kernel_main(),
 *  and then run stuff.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <common_kern.h>
#include <syscall.h>
#include <interrupt.h>
#include <vm.h>
#include <proc.h>
#include <console.h>

/* libc includes. */
#include <stdio.h>
#include <simics.h>                 /* lprintf() */

/* multiboot header file */
#include <multiboot.h>              /* boot_info */

/* x86 specific includes */
#include <x86/asm.h>                /* enable_interrupts() */

#include <x86/cr.h>
#include <malloc.h>
#include <scheduler.h>

#include <loader.h>
#include <simics.h>
#include <asm_common.h>
#include <seg.h>
#include <exception.h>
#include <malloc_wrappers.h>

#define INIT_NAME "init"
#define INIT_ARG {NULL}

#define IDLE_NAME "idle"
#define IDLE_ARG {NULL}

typedef struct {
    unsigned eip;
    unsigned cs;
    unsigned eflags;
    unsigned esp;
    unsigned ss;
} iret_args_t;

/** @brief Kernel entrypoint.
 *
 *  This is the entrypoint for the kernel.
 *
 * @return Does not return.
 */
int kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp)
{
    if (scheduler_init() < 0) {
        lprintf("failed to init scheduler");
    }

    idt_init();

    if (vm_init() < 0) {
        lprintf("failed to init vm");
    }
    set_cr0(KERNEL_CR0);

    if (proc_init() < 0) {
        lprintf("failed to init proc");
    }

    if (exception_init() < 0) {
        lprintf("failed to init exceptions");
    }

    clear_console();

    pd_t init_pd = (pd_t)get_cr3();

    /* Setup idle */
    tcb_t *idle_tcb;
    pcb_t *idle_pcb;
    if (proc_new_process(&idle_pcb, &idle_tcb) < 0) {
        lprintf("failed to create idle");
    }
    idle_tid = idle_tcb->tid;
    cur_tid = idle_tid;

    //Create a new page directory
    if (vm_new_pd(&idle_pcb->pd) < 0) {
        lprintf("vm_new_pd failed");
    }

    //Load idle into memory
    set_cr3((unsigned)idle_pcb->pd);
    unsigned idle_eip, idle_esp;
    char *idle_arg[] = IDLE_ARG;
    if (load(IDLE_NAME, idle_arg, &idle_eip, &idle_esp) < 0) {
        lprintf("idle load fails");
    }

    iret_args_t *idle_wrap_esp = (iret_args_t*)(idle_tcb->esp0 -
                                                sizeof(iret_args_t));

    //Artificially define saved regs
    idle_tcb->regs.eip = (unsigned)iret_user; //wrapper for idle
    idle_tcb->regs.esp_offset = sizeof(iret_args_t);
    idle_tcb->regs.cr0 = KERNEL_CR0;
    idle_tcb->regs.cr2 = 0;
    idle_tcb->regs.cr3 = (unsigned)idle_pcb->pd;
    idle_tcb->regs.cr4 = get_cr4();
    idle_tcb->regs.ebp_offset = -idle_tcb->esp0;
    idle_tcb->regs.eflags = USER_EFLAGS;

    //setup arguments for wrapper
    idle_wrap_esp->eip = idle_eip;
    idle_wrap_esp->cs = SEGSEL_USER_CS;
    idle_wrap_esp->eflags = USER_EFLAGS;
    idle_wrap_esp->esp = idle_esp;
    idle_wrap_esp->ss = SEGSEL_USER_DS;

    /* Setup Thread Reaper */
    tcb_t *tr_tcb;
    pcb_t *tr_pcb;
    if (proc_new_process(&tr_pcb, &tr_tcb) < 0) {
        lprintf("failed to create thread reaper");
    }

    //Create a new page directory
    if (vm_new_pd(&tr_pcb->pd) < 0) {
        lprintf("vm_new_pd failed");
    }

    //Artificially define saved regs
    tr_tcb->regs.eip = (unsigned)thread_reaper;
    tr_tcb->regs.esp_offset = 0;
    tr_tcb->regs.cr0 = KERNEL_CR0;
    tr_tcb->regs.cr2 = 0;
    tr_tcb->regs.cr3 = (unsigned)tr_pcb->pd;
    tr_tcb->regs.cr4 = get_cr4();
    tr_tcb->regs.ebp_offset = -tr_tcb->esp0;
    tr_tcb->regs.eflags = USER_EFLAGS;

    linklist_add_head(&scheduler_queue, (void*)tr_tcb->tid);

    /* Setup init */
    tcb_t *init_tcb;
    pcb_t *init_pcb;
    if (proc_new_process(&init_pcb, &init_tcb) < 0) {
        lprintf("failed to create init");
    }
    init_pcb->pd = init_pd;
    set_cr3((unsigned)init_pd);
    unsigned init_eip, init_esp;
    char *init_arg[] = INIT_ARG;
    if (load(INIT_NAME, init_arg, &init_eip, &init_esp) < 0) {
        lprintf("init load fails");
    }

    set_esp0(init_tcb->esp0);
    cur_tid = init_tcb->tid;
    lprintf("CUR_TID");
    init_tid = init_tcb->tid;
    linklist_add_head(&scheduler_queue, (void*)init_tcb->tid);

    // set_cr0((get_cr0() & ~CR0_AM & ~CR0_WP) | CR0_PE);
    jmp_user(init_eip, init_esp);

    return -1;
}
