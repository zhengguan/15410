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
#include <idt.h>
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
#include <driver_core.h>

#define MAIN_NAME "fork_test"
#define MAIN_ARG {NULL}

/** @brief Kernel entrypoint.
 *
 *  This is the entrypoint for the kernel.
 *
 * @return Does not return
 */
int kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp)
{
    if (scheduler_init() < 0) {
        lprintf("failed to init scheduler");
    }

    idt_init();

    vm_init();
    
    proc_init();
    
    if (proc_new_process() < 0)
        lprintf("failed to make new process");

    clear_console();

    char *arg[] = MAIN_ARG;
    if (exec(MAIN_NAME, arg) < 0) {
        lprintf("Failed to run main process.");
    }

    while (1) {
        continue;
    }

    return 0;
}
