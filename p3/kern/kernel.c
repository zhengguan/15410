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

#define MAIN_NAME "sleep_test"
#define MAIN_ARG {NULL}

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

    if (proc_init() < 0) {
        lprintf("failed to init proc");
    }
    tcb_t *tcb;
    if (proc_new_process(NULL, &tcb) < 0) {
        lprintf("failed to create new process");
    }

    set_esp0(tcb->esp0);
    cur_tid = tcb->tid;
    linklist_add_tail(&scheduler_queue, (void *)tcb->tid);


    clear_console();

    char *arg[] = MAIN_ARG;
    if (load(MAIN_NAME, arg, true) < 0) {
        lprintf("Failed to run main process.");
    }

    while (1) {
        continue;
    }

    return 0;
}
