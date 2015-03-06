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

#define INT_STACK_SIZE (2 * PAGE_SIZE)

#define MAIN_NAME "ck1"
#define MAIN_ARG {NULL}

/** @brief Kernel entrypoint.
 *
 *  This is the entrypoint for the kernel.
 *
 * @return Does not return
 */
int kernel_main(mbinfo_t *mbinfo, int argc, char **argv, char **envp)
{
    set_esp0((unsigned)malloc(INT_STACK_SIZE));

    idt_init();
    
    vm_init();

    proc_init();
    proc_new_process();

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
