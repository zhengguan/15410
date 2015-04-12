/** @file asm_exception.S
 *  @brief Contains wrappers for exception handlers.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <ureg.h>
#include <idt.h>

.macro EXN_WRAPPER NAME CAUSE
.globl \NAME
\NAME:
    push    $0
    pusha
    push    %gs
    push    %fs
    push    %es
    push    %ds
    mov     %cr2, %eax
    push    %eax
    push    $\CAUSE
    call    set_kernel_segs
    call    exception_handler
    call    set_user_segs
    addl    $4, %esp
.endm

.macro EXN_WRAPPER_ERR NAME CAUSE
.globl \NAME
\NAME:
    pusha
    push    %gs
    push    %fs
    push    %es
    push    %ds
    mov     %cr2, %eax
    push    %eax
    push    $\CAUSE
    call    set_kernel_segs
    call    exception_handler
    call    set_user_segs
    addl    $4, %esp
.endm

EXN_WRAPPER exn_divide_wrapper, SWEXN_CAUSE_DIVIDE
EXN_WRAPPER exn_debug_wrapper, SWEXN_CAUSE_DEBUG
EXN_WRAPPER exn_breakpoint_wrapper, SWEXN_CAUSE_BREAKPOINT
EXN_WRAPPER exn_overflow_wrapper, SWEXN_CAUSE_OVERFLOW
EXN_WRAPPER exn_boundcheck_wrapper, SWEXN_CAUSE_BOUNDCHECK
EXN_WRAPPER exn_opcode_wrapper, SWEXN_CAUSE_OPCODE
EXN_WRAPPER exn_nofpu_wrapper, SWEXN_CAUSE_NOFPU
EXN_WRAPPER exn_fpufault_wrapper, SWEXN_CAUSE_FPUFAULT
EXN_WRAPPER exn_simdfault_wrapper, SWEXN_CAUSE_SIMDFAULT
EXN_WRAPPER exn_nmi_wrapper, IDT_NMI
EXN_WRAPPER exn_cso_wrapper, IDT_CSO
EXN_WRAPPER exn_mc_wrapper, IDT_MC

// Hardware pushes error code for below exceptions
EXN_WRAPPER_ERR exn_segfault_wrapper, SWEXN_CAUSE_SEGFAULT
EXN_WRAPPER_ERR exn_stackfault_wrapper, SWEXN_CAUSE_STACKFAULT
EXN_WRAPPER_ERR exn_protfault_wrapper, SWEXN_CAUSE_PROTFAULT
EXN_WRAPPER_ERR exn_pagefault_wrapper, SWEXN_CAUSE_PAGEFAULT
EXN_WRAPPER_ERR exn_alignfault_wrapper, SWEXN_CAUSE_ALIGNFAULT
EXN_WRAPPER_ERR exn_ts_wrapper, IDT_TS

