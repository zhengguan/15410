/** @file exception.h
 *  @brief This file defines the interface for exception functions.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#include <syscall.h>
#include <proc.h>

void deregister_swexn_handler(pcb_t *pcb);
int register_swexn_handler(pcb_t *pcb, swexn_handler_t eip, void *esp3, void *arg);
int dup_swexn_handler(pcb_t *src_pcb, pcb_t *dest_pcb);

#endif /* _EXCEPTION_H */
