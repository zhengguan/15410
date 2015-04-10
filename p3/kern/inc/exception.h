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

int exception_init();
int deregister_swexn_handler(int pid);
int register_swexn_handler(int pid, swexn_handler_t eip, void *esp3, void *arg);
int dup_swexn_handler(int src_pid, int dest_pid);

#endif /* _EXCEPTION_H */
