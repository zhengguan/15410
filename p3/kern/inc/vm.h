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
#include <memlock.h>

#define PTE_PRESENT 0x1
#define PTE_RW 0x2
#define PTE_SU 0x4
#define PTE_GLOBAL 0x100

#define KERNEL_FLAGS (PTE_PRESENT | PTE_RW | PTE_GLOBAL)
#define USER_FLAGS_RO (PTE_PRESENT | PTE_SU)
#define USER_FLAGS_RW (PTE_PRESENT | PTE_RW | PTE_SU)

typedef unsigned pde_t;
typedef unsigned pte_t;
typedef pde_t* pd_t;
typedef pte_t* pt_t;

extern memlock_t vm_memlock;

/* Virtual memory functions */
int vm_init();
int vm_new_pd();
int vm_copy(pd_t *new_pd);
void vm_clear();
void vm_destroy();
void vm_read_only();
void vm_read_write(void *va);
void vm_super(void *va);
void vm_user(void *va);
bool vm_check_flags(void *va, unsigned flags);
bool vm_check_flags_len(void *base, int len, unsigned flags);
bool vm_lock(void *va, unsigned flags);
bool vm_lock_len(void *base, int len, unsigned flags);
int vm_lock_str(char *str, unsigned flags);
void vm_unlock(void *va);
void vm_unlock_len(void *base, int len);
void vm_phys_write(unsigned pa, unsigned val);
unsigned vm_phys_read(unsigned pa);


#endif /* _VM_H */
