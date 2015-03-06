/** @file vm.c
 *  @brief Handles virtual memory.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <vm.h>
#include <x86/cr.h>
#include <stdlib.h>
#include <malloc.h>
#include <syscall.h>
#include <common_kern.h>
#include <linklist.h>

unsigned next_frame = USER_MEM_START;
// TODO Improve space complexity of this data structure
linklist_t free_frames;

/** @brief Gets a free physical frame.
 *
 *  @return Physical address of the frame.
 */
static unsigned get_frame()
{
    // TODO add locking mechanism?
    unsigned frame;
    if (linklist_empty(&free_frames)) {
        frame = next_frame;
        next_frame += PAGE_SIZE;
    } else {
        linklist_remove_head(&free_frames, (void **)&frame);
    }
    return frame;
}

/** @brief Checks whether a virtual address in the page table.
 *
 *  @return True if present, false otherwise.
 */
static int is_present(void *va)
{
    pde_t pde = GET_PDE(va);
    if (GET_PRESENT(pde)) {
        pte_t pte = GET_PTE(pde, va);
        if (GET_PRESENT(pte)) {
            return TRUE;
        }
    }

    return FALSE;
}

/** @brief Initializes the virtual memory.
 *
 *  @return Void.
 */
void vm_init()
{
    linklist_init(&free_frames);
    
    vm_new_pd();
    
    set_cr0(get_cr0() | CR0_PG);
}

/** @brief Creates a new page directory and sets the %cr3 register.
 *
 *  @return Void.
 */
void vm_new_pd()
{
    // TODO maybe move PDs to top of address space rather than in kernel mem   
    
    pd_t pd = smemalign(PAGE_SIZE, PAGE_SIZE);
    // FIXME check for failure
    set_cr3((unsigned)pd);

    int i;
    for (i = 0; i < PD_SIZE; i++) {
        pd[i] = SET_PRESENT(pd[i], PTE_PRESENT_NO);
    }

    unsigned addr;
    for(addr = KERNEL_MEM_START; addr < USER_MEM_START; addr += PAGE_SIZE) {
        vm_new_pte((void *)addr, addr, PTE_RW_WRITE | PTE_SU_SUPER);
    }

    // FIXME is this necessary
    pde_t *pde = &GET_PDE(pd);
    *pde |= PTE_SU_USER;
}

/** @brief Creates a new page directory entry.
 *
 *  @param pde The page directory entry.
 *  @param pt The page table for the page directory entry.
 *  @param su The page directory entry flags.
 *  @return Physical address of base of the new page table.
 */
void vm_new_pde(pde_t *pde, pt_t pt, unsigned flags)
{
    *pde = (ROUND_DOWN_PAGE(pt) | PTE_PRESENT_YES | flags);
}

/** @brief Creates a new page table.
 *
 *  @param pde The page directory entry for the new page table.
 *  @param su The page directory entry flags for the new page table.
 *  @return Physical address of base of the new page table.
 */
void vm_new_pt(pde_t *pde, unsigned flags)
{
    pt_t pt = smemalign(PAGE_SIZE, PAGE_SIZE);
    // FIXME check for failure

    int i;
    for (i = 0; i < PT_SIZE; i++) {
        pt[i] &= SET_PRESENT(pt[i], PTE_PRESENT_NO);
    }

    vm_new_pde(pde, pt, flags);
}

/** @brief Creates a new page table entry for a virtual address.
 *
 *  @param va The virtual address.
 *  @param pa The mapped physical address.
 *  @param su The page table entry flags.
 *  @return Physical address of base of the new page table.
 */
void vm_new_pte(void *va, unsigned pa, unsigned flags)
{
    pde_t *pde = &GET_PDE(va);

    if (!GET_PRESENT(*pde)) {
        vm_new_pt(pde, flags);
    }

    pte_t *pte = &GET_PTE(*pde, va);
    *pte = (ROUND_DOWN_PAGE(pa) | PTE_PRESENT_YES | flags);
}

/** @brief Allocated memory starting at base and extending for len bytes.
 *
 *  @param base The base of the memory region to allocate.
 *  @param len The number of bytes to allocate.
 *  @return 0 on success, negative error code otherwise.
 */
int new_pages(void *base, int len)
{
    if (((unsigned)base % PAGE_SIZE) != 0) {
        return -1;
    }

    if ((len % PAGE_SIZE) != 0) {
        return -2;
    }

    void *va;
    for (va = base; va < base + len - 1; va += PAGE_SIZE) {
        if (is_present(va)) {
            return -3;
        }
    }
    
    for (va = base; va < base + len - 1; va += PAGE_SIZE) {
        vm_new_pte(va, get_frame(), PTE_RW_WRITE | PTE_SU_USER);
    }

    return 0;
}

/** @brief Deallocate the memory region starting at base.
 *
 *  @param base The base of the memory region to deallocate.
 *  @return 0 on success, negative error code otherwise.
 */
int remove_pages(void *base)
{
    // TODO How to keep track of allocated region len?
    return 0;
}
