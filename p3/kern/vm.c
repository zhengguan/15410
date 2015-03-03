/** @file vm.c
 *  @brief Handles virtual memory.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <vm.h>
#include <syscall.h>
#include <linklist.h>
#include <cr3.h>

unsigned next_phy_frame = FIRST_PHY_FRAME;
linklist_t free_frames;

/** @brief Initializes the virtual memory.
 *
 *  @return Void.
 */
void vm_init() {
    linklist_init(&free_frames);
    
    set_cr3(vm_pd_init());
}

/** @brief Initializes a new page directory.
 *
 *  @return Physical address of base of the new page directory.
 */
void *vm_pd_init() {
    // TODO add locking mechanism?
    
    pd_t pd = get_free_frame();
    
    // TODO what to do here?
    pd[0] = (((unsigned)vm_pt_init() & BASE_ADDR_MASK) |
        PTE_PRESENT_YES | PTE_RW_WRITE | PTE_SU_SUPER);
    
    int i;
    for (i = 1; i < PD_SIZE; i++) {
        // TODO what to do here?
        pd[i] |= (PTE_PRESENT_NO | PTE_RW_WRITE | PTE_SU_SUPER);
    }
    
    return (void *)pd;
}

/** @brief Initializes a new page table.
 *
 *  @return Physical address of base of the new page table.
 */
void *vm_pt_init() {
    pt_t pt = get_free_frame();
    
    int i;
    for (i = 0; i < PD_SIZE; i++) {
        // TODO what to do here?
        pt[i] |= (PTE_PRESENT_NO | PTE_RW_WRITE | PTE_SU_SUPER);
    }
    
    return (void *)pt;
}

/** @brief Gets a free physical frame.
 *
 *  @return Physical address of the frame.
 */
static void *get_free_frame() {
    void *frame;
    if (linklist_empty(&free_frames)) {
        frame = (void *)next_phy_frame;
        next_phy_frame += PAGE_SIZE;
    } else {
        linklist_remove_head(&free_frames, &frame);
    }
    return frame;
}

/** @brief Allocated memory starting at base and extending for len bytes.
 *
 *  @param base The base of the memory region to allocate.
 *  @param len The number of bytes to allocate.
 *  @return 0 on success, negative error code otherwise.
 */
int new_pages(void *base, int len) {
    if (((unsigned)base % PAGE_SIZE) != 0) {
        return -1;
    }
    
    if ((len % PAGE_SIZE) != 0) {
        return -2;
    }

    pd_t pd = get_cr3();
    
    // TODO find better solution for this looping
    
    int num_pages = len / PAGE_SIZE;
    pte_t *ptes[num_pages];
    
    int i = 0;
    while (i < num_pages) {
        pt_t pt = (pt_t)(pd[GET_PD_IDX(base)] & BASE_ADDR_MASK);
        int pt_idx = GET_PT_IDX(base);
        while(i < num_pages) {
            pte_t pte = pt[pt_idx++];
            if (!(pte & PTE_PRESENT_YES)) {
                return -3;
            };
            ptes[i++] = &pte;
        }
    }
    
    for (i = 0; i < num_pages; i++) {
        *ptes[i] = (((unsigned)get_free_frame() & BASE_ADDR_MASK) |
            PTE_PRESENT_YES | PTE_RW_WRITE | PTE_SU_SUPER);
    }
    
    return 0;
}

/** @brief Deallocate the memory region starting at base.
 *
 *  @param base The base of the memory region to deallocate.
 *  @return 0 on success, negative error code otherwise.
 */
int remove_pages(void *base) {
    // TODO How to keep track of allocated region len?
    return 0;
}
