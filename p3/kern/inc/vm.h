/** @file vm.h
 *  @brief Prototypes for managing virtual memory.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _VM_H
#define _VM_H

#include <kern_common.h>

typedef unsigned pde_t;
typedef unsigned pte_t;
typedef pde_t* pd_t;
typedef pte_t* pt_t;

/* Virtual memory functions */
bool vm_is_present(void *va);
bool vm_is_present_len(void *base, unsigned len);
int vm_init();
int vm_new_pd();
bool vm_is_present(void *va);
int vm_copy(pd_t *new_pd);
void vm_clear();
void vm_read_only();
void vm_read_write();
void vm_destroy();

#endif /* _VM_H */
