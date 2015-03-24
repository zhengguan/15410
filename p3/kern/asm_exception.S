/** @file asm_exception.S
 *  @brief Contains wrappers for exception handlers.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <ureg.h>

.macro MAKE_WRAPPER NAME CAUSE
.globl \NAME
\NAME:
    push $0
    pusha
    push %gs
    push %fs
    push %es
    push %ds
    mov %cr2, %eax
    push %eax
    push $\CAUSE
    call set_kernel_segs
    call exception_handler
    call set_user_segs
    addl $4, %esp
.endm

.macro MAKE_WRAPPER_ERR NAME CAUSE
.globl \NAME
\NAME:
    pusha
    push %gs
    push %fs
    push %es
    push %ds
    mov %cr2, %eax
    push %eax
    push $\CAUSE
    call set_kernel_segs
    call exception_handler
    call set_user_segs
    addl $4, %esp
.endm

MAKE_WRAPPER exn_divide_wrapper, SWEXN_CAUSE_DIVIDE
MAKE_WRAPPER exn_debug_wrapper, SWEXN_CAUSE_DEBUG
MAKE_WRAPPER exn_breakpoint_wrapper, SWEXN_CAUSE_BREAKPOINT
MAKE_WRAPPER exn_overflow_wrapper, SWEXN_CAUSE_OVERFLOW
MAKE_WRAPPER exn_boundcheck_wrapper, SWEXN_CAUSE_BOUNDCHECK
MAKE_WRAPPER exn_opcode_wrapper, SWEXN_CAUSE_OPCODE
MAKE_WRAPPER exn_nofpu_wrapper, SWEXN_CAUSE_NOFPU
MAKE_WRAPPER exn_fpufault_wrapper, SWEXN_CAUSE_FPUFAULT
MAKE_WRAPPER exn_simdfault_wrapper, SWEXN_CAUSE_SIMDFAULT
// Hardware pushes error code for below exceptions
MAKE_WRAPPER_ERR exn_segfault_wrapper, SWEXN_CAUSE_SEGFAULT
MAKE_WRAPPER_ERR exn_stackfault_wrapper, SWEXN_CAUSE_STACKFAULT
MAKE_WRAPPER_ERR exn_protfault_wrapper, SWEXN_CAUSE_PROTFAULT
MAKE_WRAPPER_ERR exn_pagefault_wrapper, SWEXN_CAUSE_PAGEFAULT
MAKE_WRAPPER_ERR exn_alignfault_wrapper, SWEXN_CAUSE_ALIGNFAULT
