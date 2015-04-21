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

void deregister_swexn_handler(tcb_t *tcb);
void register_swexn_handler(tcb_t *tcb, swexn_handler_t eip, void *esp3,
  void *arg);
void dup_swexn_handler(tcb_t *src_tcb, tcb_t *dest_tcb);

#endif /* _EXCEPTION_H */
