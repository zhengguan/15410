/** @file vm.h
 *  @brief Prototypes for managing virtual memory.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef __VM_H__
#define __VM_H__

#include <macros.h>

#define KERNEL_MEM_START 0x00000000

#define PTE_PRESENT_NO 0x0
#define PTE_PRESENT_YES 0x1
#define PTE_RW_READ 0x0
#define PTE_RW_WRITE 0x2
#define PTE_SU_SUPER 0x0
#define PTE_SU_USER 0x4

#define PTE_PRESENT_MASK 0xFFFFFFFE
#define GET_PRESENT(PTE) ((PTE) & PTE_PRESENT_YES)
#define SET_PRESENT(PTE,PRESENT) (((unsigned)(PTE) & PTE_PRESENT_MASK) | PRESENT)

#define PD_SIZE (PAGE_SIZE / sizeof(pde_t))
#define PT_SIZE (PAGE_SIZE / sizeof(pte_t))

#define PD_MASK 0xFFC00000
#define PD_SHIFT 22
#define GET_PD() ((pd_t)get_cr3())
#define GET_PD_IDX(ADDR) (((unsigned)(ADDR) & PD_MASK) >> PD_SHIFT)
#define GET_PDE(PD, ADDR) ((PD)[GET_PD_IDX(ADDR)])

#define PT_MASK 0x003FF000
#define PT_SHIFT 12
#define GET_PT(PDE) ((pt_t)((PDE) & PAGE_MASK))
#define GET_PT_IDX(ADDR) (((unsigned)(ADDR) & PT_MASK) >> PT_SHIFT)
#define GET_PTE(PDE, ADDR) (GET_PT(PDE)[GET_PT_IDX(ADDR)])

#define GET_VA(PD_IDX, PT_IDX) ((PD_IDX << PD_SHIFT) || (PT_IDX << PT_SHIFT))

#define PAGES_HT_SIZE 128

#define PAGE_NUM(ADDR) ((unsigned)(ADDR) >> PT_SHIFT)

#define FLAG_MASK (~PAGE_MASK & ~1)
#define GET_FLAGS(PDE) ((PDE) & FLAG_MASK)
#define GET_PA(PTE) ((PTE) & PAGE_MASK)

typedef unsigned pde_t;
typedef unsigned pte_t;
typedef pde_t* pd_t;
typedef pte_t* pt_t;

/* Virtual memory functions */
void vm_init();
pd_t vm_new_pd();
void vm_new_pde(pde_t *pde, pt_t pt, unsigned flags);
pt_t vm_new_pt(pde_t *pde, unsigned flags);
void vm_new_pte(pd_t pd, void *va, unsigned pa, unsigned flags);
void vm_remove_pd();
void vm_remove_pde(pde_t *pde);
void vm_remove_pt(pde_t *pde);
unsigned vm_remove_pte(void *va);
unsigned vm_copy();
bool vm_is_present(void *va);

#endif /* __VM_H__ */