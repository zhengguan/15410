#ifndef _ASM_EXCEPTION_H_
#define _ASM_EXCEPTION_H_

#include <ureg.h>

void exn_divide_wrapper();
void exn_debug_wrapper();
void exn_breakpoint_wrapper();
void exn_overflow_wrapper();
void exn_boundcheck_wrapper();
void exn_opcode_wrapper();
void exn_nofpu_wrapper();
void exn_segfault_wrapper();
void exn_stackfault_wrapper();
void exn_protfault_wrapper();
void exn_pagefault_wrapper();
void exn_fpufault_wrapper();
void exn_alignfault_wrapper();
void exn_simdfault_wrapper();

void exn_nmi_wrapper();
void exn_cso_wrapper();
void exn_mc_wrapper();
void exn_ts_wrapper();

#endif /* _ASM_EXCEPTION_H_ */
