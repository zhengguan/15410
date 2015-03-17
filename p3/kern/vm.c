/** @file vm.c
 *  @brief Handles virtual memory.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <vm.h>
#include <x86/cr.h>
#include <malloc.h>
#include <syscall.h>
#include <common_kern.h>
#include <linklist.h>
#include <hashtable.h>
#include <macros.h>
#include <simics.h>

unsigned next_frame = USER_MEM_START;
// TODO Improve space complexity of this data structure
linklist_t free_frames;
hashtable_t alloc_pages;

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
static bool is_present(void *va)
{
    pde_t pde = GET_PDE(va);
    if (GET_PRESENT(pde)) {
        pte_t pte = GET_PTE(pde, va);
        if (GET_PRESENT(pte)) {
            return true;
        }
    }

    return false;
}

/** @brief Checks whether the page table referenced by a page directory entry
 *  is empty.
 *
 *  @param pde The page directory entry.
 *  @return True if empty, false otherwise.
 */
static bool is_pt_empty(pde_t *pde) {
    pt_t pt = GET_PT(*pde);

    int i;
    for (i = 0; i < PT_SIZE; i++) {
        if (GET_PRESENT(pt[i])) {
            return false;
        }
    }
    
    return true;
}

/** @brief Initializes the virtual memory.
 *
 *  @return Void.
 */
void vm_init()
{
    linklist_init(&free_frames);
    hashtable_init(&alloc_pages, PAGES_HT_SIZE);
    
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

/** @brief Removes the page directory.
 *
 *  @return Void.
 */
void vm_remove_pd() {
    // TODO do we need this? Probably on exiting
}

/** @brief Removes a page directory entry.
 *
 *  @param pde The page directory entry.
 *  @return Void.
 */
void vm_remove_pde(pde_t *pde) {
    *pde = SET_PRESENT(*pde, PTE_PRESENT_NO);
}

/** @brief Removes a page table.
 *
 *  @param pde The page directory entry of the page table to be removed.
 *  @return Void.
 */
void vm_remove_pt(pde_t *pde) {
    pt_t pt = GET_PT(*pde);
    sfree(pt, PAGE_SIZE);
    linklist_add_tail(&free_frames, (void *)pt);
    
    vm_remove_pde(pde); 
}

/** @brief Removes a page table entry for a virtual address.
 *
 *  @param va The virtual address.
 *  @return Mapped physical address of the removed page table.
 */
unsigned vm_remove_pte(void *va) {
    // TODO maybe possible race condition here

    pde_t *pde = &GET_PDE(va);
    pte_t *pte = &GET_PTE(*pde, va);
    *pte = SET_PRESENT(*pte, PTE_PRESENT_NO);
    
    if (is_pt_empty(pde)) {
        vm_remove_pt(pde);
    }
    
    return ROUND_DOWN_PAGE(*pte);
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
    
    hashtable_add(&alloc_pages, PAGE_NUM(base), (void *)len);

    return 0;
}

/** @brief Deallocate the memory region starting at base.
 *
 *  @param base The base of the memory region to deallocate.
 *  @return 0 on success, negative error code otherwise.
 */
int remove_pages(void *base)
{
    return 0;
    
    if (((unsigned)base % PAGE_SIZE) != 0) {
        return -1;
    }
    
    int len;
    if (!hashtable_get(&alloc_pages, PAGE_NUM(base), (void**)&len)) {
        return -2;
    }
    
    hashtable_remove(&alloc_pages, PAGE_NUM(base));
    
    void *va;
    for (va = base; va < base + len - 1; va += PAGE_SIZE) {
        unsigned pa = vm_remove_pte(va);
        linklist_add_tail(&free_frames, (void *)pa); 
    }
    
    return 0;
}
