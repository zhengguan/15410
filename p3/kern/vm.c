/** @file vm.c
 *  @brief Handles virtual memory.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <vm.h>
#include <linklist.h>
#include <syscall.h>
#include <x86/cr.h>
#include <common_kern.h>

unsigned next_frame = USER_MEM_START;
// TODO Improve space complexity of this data structure
linklist_t free_frames;

/** @brief Gets a free physical frame.
 *
 *  @return Physical address of the frame.
 */
static unsigned get_frame()
{
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
    if (IS_PRESENT(pde)) {
        pte_t pte = GET_PTE(pde, va);
        if (IS_PRESENT(pte)) {
            return 1;
        }
    }
    
    return 0;
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

/** @brief Initializes a new page directory.
 *
 *  @return Physical address of base of the new page directory.
 */
unsigned vm_new_pd()
{
    // TODO add locking mechanism?
    
    pd_t pd = (pd_t)get_frame();
    set_cr3((unsigned)pd);
    
    int i;
    for (i = 0; i < PD_SIZE; i++) {
        pd[i] &= PTE_PRESENT_NO;
    }
    
    unsigned va;
    for(va = KERNEL_MEM_START; va < USER_MEM_START; va += PAGE_SIZE) {
        vm_new_pte((void *)va, va, PTE_SU_SUPER);
    }
    
    vm_new_pte((void *)pd, (unsigned)pd, PTE_SU_SUPER);
    
    return (unsigned)pd;
}

/** @brief Initializes a new page directory entry.
 *
 *  @param pde The page directory entry.
 *  @param pt The page table for the page directory entry.
 *  @return Physical address of base of the new page table.
 */
void vm_new_pde(pde_t *pde, pt_t pt)
{
    *pde = (((unsigned)pt & BASE_ADDR_MASK) | PTE_PRESENT_YES | PTE_RW_WRITE);
}

/** @brief Initializes a new page table.
 *
 *  @param pde The page directory entry for the new page table.
 *  @return Physical address of base of the new page table.
 */
void vm_new_pt(pde_t *pde)
{    
    pt_t pt = (pt_t)get_frame();
    
    int i;
    for (i = 0; i < PT_SIZE; i++) {
        pt[i] &= PTE_PRESENT_NO;
    }
    
    vm_new_pde(pde, pt);
    
    vm_new_pte((void *)pt, (unsigned)pt, PTE_SU_SUPER);
}

/** @brief Initializes a new page table entry for a virtual address.
 *
 *  @param va The virtual address.
 *  @param pa The mapped physical address.
 *  @param su The page table entry su flag.
 *  @return Physical address of base of the new page table.
 */
void vm_new_pte(void *va, unsigned pa, unsigned su)
{
    pde_t *pde = &GET_PDE(va);
    
    if (!IS_PRESENT(*pde)) {
        vm_new_pt(pde);
    }
    
    pte_t *pte = &GET_PTE(*pde, va);
    *pte = ((pa & BASE_ADDR_MASK) | PTE_PRESENT_YES |
                             PTE_RW_WRITE | su);
}


/** @brief Allocated memory starting at base and extending for len bytes.
 *
 *  @param base The base of the memory region to allocate.
 *  @param len The number of bytes to allocate.
 *  @return 0 on success, negative error code otherwise.
 */
int new_pages(void *base, int len)
{
    if (((uint32_t)base % PAGE_SIZE) != 0) {
        return -1;
    }
    
    if ((len % PAGE_SIZE) != 0) {
        return -2;
    }
    
    void *va;
    for (va = base; va < base + len; va += PAGE_SIZE) { 
        if (is_present(va)) {
            return -3;
        }
    }
    
    for (va = base; va < base + len; va += PAGE_SIZE) { 
        vm_new_pte(va, get_frame(), PTE_SU_USER);
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
