/** @file vm.c
 *  @brief Manages virtual memory.
 *
 *  Manages virtual memory using a two-level page table structure.  Implements
 *  new_pages and remove_pages system calls.  Free physical frames are kept in
 *  a linked list.  The length of memory regions allocated by calls to
 *  new_pages are kept in a hashtable.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <vm.h>
#include <x86/cr.h>
#include <malloc.h>
#include <string.h>
#include <syscall.h>
#include <common_kern.h>
#include <linklist.h>
#include <hashtable.h>
#include <simics.h>
#include <string.h>
#include <kern_common.h>

#define KERNEL_MEM_START 0x00000000

#define PTE_PRESENT 0x1
#define PTE_RW 0x2
#define PTE_SU 0x4
#define PTE_GLOBAL 0x100

#define FLAG_MASK (~PAGE_MASK & ~1)
#define GET_PRESENT(PTE) ((PTE) & PTE_PRESENT)
#define GET_FLAGS(PTE) ((PTE) & FLAG_MASK)

#define KERNEL_FLAGS (PTE_RW | PTE_GLOBAL)
#define USER_FLAGS (PTE_RW | PTE_SU)

#define PD_SIZE (PAGE_SIZE / sizeof(pde_t))
#define PT_SIZE (PAGE_SIZE / sizeof(pte_t))

#define PD_MASK 0xFFC00000
#define PD_SHIFT 22
#define GET_PD() ((pd_t)get_cr3())
#define GET_PD_IDX(ADDR) (((unsigned)(ADDR) & PD_MASK) >> PD_SHIFT)
#define GET_PDE(PD, ADDR) ((pde_t)((PD)[GET_PD_IDX(ADDR)]))

#define PT_MASK 0x003FF000
#define PT_SHIFT 12
#define GET_PT(PDE) ((pt_t)((PDE) & PAGE_MASK))
#define GET_PT_IDX(ADDR) (((unsigned)(ADDR) & PT_MASK) >> PT_SHIFT)
#define GET_PTE(PDE, ADDR) ((pte_t)(GET_PT(PDE)[GET_PT_IDX(ADDR)]))

#define GET_PA(PTE) ((unsigned)(PTE) & PAGE_MASK)

#define PAGES_HT_SIZE 128
#define LOOKUP_PA(ADDR) (GET_PA(GET_PTE(GET_PDE(GET_PD(), ADDR), ADDR)))
#define PAGE_NUM(ADDR) (LOOKUP_PA(ADDR) >> PT_SHIFT)

int vm_new_pde(pde_t *pde, pt_t pt, unsigned flags);
int vm_new_pt(pde_t *pde, unsigned flags);
int vm_new_pte(pd_t pd, void *va, unsigned pa, unsigned flags);
void vm_remove_pde(pde_t *pde);
void vm_remove_pt(pde_t *pde);
void vm_remove_pte(void *va);

unsigned next_frame = USER_MEM_START;
linklist_t free_frames;
hashtable_t alloc_pages;

/** @brief Gets a free physical frame.
 *
 *  Returns a previously allocated free physical frame if one is available.
 *  Otherwise it gets a new free physical frame.
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
 *  Creates a new page table directory and sets %cr3, sets the paging bit in
 *  %cr0, and sets the page global enable bit in %cr4.
 *
 *  @return 0 on success, negative error code otherwise.
 */
int vm_init()
{
    if (linklist_init(&free_frames) < 0) {
        return -1;
    }

    if (hashtable_init(&alloc_pages, PAGES_HT_SIZE) < 0) {
        return -2;
    }

    pd_t pd;
    if (vm_new_pd(&pd) < 0) {
        return -3;
    }
    set_cr3((unsigned)pd);

    set_cr0(get_cr0() | CR0_PG);
    set_cr4(get_cr4() | CR4_PGE);

    return 0;
}

/** @brief Creates a new page directory.
 *
 *  Allocates a new page dirctory, clears all present bits, and direct maps
 *  the kernel memory region.
 *
 *  @param new_pd A location in memory to store the physical address of the new
 *  page directory.
 *  @return 0 on success, negative error code otherwise.
 */
int vm_new_pd(pd_t *new_pd)
{
    pd_t pd = smemalign(PAGE_SIZE, PAGE_SIZE);
    if (pd == NULL) {
        return -1;
    }

    int i;
    for (i = 0; i < PD_SIZE; i++) {
        pd[i] &= ~PTE_PRESENT;
    }

    unsigned pa;
    for (pa = KERNEL_MEM_START; pa < USER_MEM_START; pa += PAGE_SIZE) {
        vm_new_pte(pd, (void *)pa, pa, KERNEL_FLAGS);
    }

    *new_pd = pd;

    return 0;
}

/** @brief Creates a new page directory entry.
 *
 *  @param pde The page directory entry.
 *  @param pt The page table for the page directory entry.
 *  @param flags The page directory entry flags.
 *  @return 0 on success, negative error code otherwise.
 */
int vm_new_pde(pde_t *pde, pt_t pt, unsigned flags)
{
    *pde = (GET_PA(pt) | PTE_PRESENT | flags);

    return 0;
}

/** @brief Creates a new page table.
 *
 *  Also creates a new page directory entry for the new page table.
 *
 *  @param pde The page directory entry for the new page table.
 *  @param flags The page directory entry flags for the new page table.
 *  @return 0 on success, negative error code otherwise.
 */
int vm_new_pt(pde_t *pde, unsigned flags)
{
    pt_t pt = smemalign(PAGE_SIZE, PAGE_SIZE);
    if (pt == NULL) {
        return -1;
    }

    int i;
    for (i = 0; i < PT_SIZE; i++) {
        pt[i] &= ~PTE_PRESENT;
    }

    vm_new_pde(pde, pt, flags);

    return 0;
}


/** @brief Creates a new page table entry for a virtual address.
 *
 *  Creates a new page table and page directory entry if necessary.
 *
 *  @param pd The page directory for the new page table entry.
 *  @param va The virtual address.
 *  @param pa The mapped physical address.
 *  @param flags The page table entry flags.
 *  @return 0 on success, negative error code otherwise.
 */
int vm_new_pte(pd_t pd, void *va, unsigned pa, unsigned flags)
{
    pde_t *pde = pd + GET_PD_IDX(va);
    if (!GET_PRESENT(*pde)) {
        if (vm_new_pt(pde, flags) < 0) {
            return -1;
        }
    }

    pte_t *pte = GET_PT(*pde) + GET_PT_IDX(va);
    *pte = ((pa & PAGE_MASK) | PTE_PRESENT | flags);

    return 0;
}

/** @brief Removes a page directory entry.
 *
 *  Clears the present bit on the page directory entry.
 *
 *  @param pde The page directory entry.
 *  @return Void.
 */
void vm_remove_pde(pde_t *pde) {
    *pde &= ~PTE_PRESENT;
}

/** @brief Removes a page table.
 *
 *  Also removes the page directory entry for the remove page table.
 *
 *  @param pde The page directory entry of the page table to be removed.
 *  @return Void.
 */
void vm_remove_pt(pde_t *pde) {
    pt_t pt = GET_PT(*pde);
    sfree(pt, PAGE_SIZE);
    vm_remove_pde(pde);
}

/** @brief Removes a page table entry for a virtual address.
 *
 *  Clears the present bit on the page table entry, adds the physical frame to
 *  the list of free frames, and removes the page table and page directory
 *  entry if necessary.
 *
 *  @param va The virtual address.
 *  @return Void.
 */
void vm_remove_pte(void *va) {
    if (!vm_is_present(va)) {
        return;
    }

    pde_t *pde = GET_PD() + GET_PD_IDX(va);

    pte_t *pte =  GET_PT(*pde) + GET_PT_IDX(va);
    *pte &= ~PTE_PRESENT;

    unsigned pa = GET_PA(*pte);
    if (pa >= USER_MEM_START) {
        linklist_add_tail(&free_frames, (void *)(*pte & PAGE_MASK));
    }

    if (is_pt_empty(pde)) {
        vm_remove_pt(pde);
    }
}

/** @brief Checks whether a virtual address in the page table.
 *
 *  @return True if present, false otherwise.
 */
bool vm_is_present(void *va)
{
    va = (void*)ROUND_DOWN_PAGE(va);
    pde_t pde = GET_PDE(GET_PD(), va);
    if (GET_PRESENT(pde)) {
        pte_t pte = GET_PTE(pde, va);
        if (GET_PRESENT(pte)) {
            return true;
        }
    }

    return false;
}

/** @brief Checks whether a memory region is the page table.
 *
 *  @param base The base of the memory region to allocate.
 *  @param len The number of bytes to allocate.
 *  @return True if present, false otherwise.
 */
bool vm_is_present_len(void *base, unsigned len)
{

    if ((unsigned)base > -len)
        return false;

    unsigned va = (unsigned)base;

    if (va > va + len - 1) {
        return false;
    }

    for (;  va < (unsigned)base + len - 1; va += PAGE_SIZE) {
        if (!vm_is_present((void*)va)) {
            return false;
        }
    }

    return true;
}

/** @brief Copies the virtual address space into new page directory.
 *
 *  Create a new page directory and iterate over the address space and copy all
 *  present page table entries into the new page directory.
 *
 *  @return 0 on success, negative error code otherwise.
 */
int vm_copy(pd_t *new_pd)
{
    pd_t old_pd = GET_PD();
    if (vm_new_pd(new_pd) < 0) {
        return -1;
    }

    char *buf = malloc(PAGE_SIZE);
    if (buf == NULL) {
        return -2;
    }

    char *va = (char*)USER_MEM_START;
    while ((unsigned)va >= USER_MEM_START) {
        pde_t pde = GET_PDE(old_pd, va);
        if (!GET_PRESENT(pde)) {
            va += (1 << PD_SHIFT);
            continue;
         }

        pte_t pte = GET_PTE(pde, va);
        if (!GET_PRESENT(pte)) {
            va += (1 << PT_SHIFT);
            continue;
        }

        vm_new_pte(*new_pd, va, get_frame(), GET_FLAGS(pte));

        memcpy(buf, va, PAGE_SIZE);
        set_cr3((unsigned)*new_pd);
        memcpy(va, buf, PAGE_SIZE);
        set_cr3((unsigned)old_pd);

        va += PAGE_SIZE;
    }

    free(buf);

    return 0;
}

/** @brief Clear the virtual address space of all user memory.
 *
 *  Iterate over the address space and remove all present page table entries
 *  which will also remove all page tables and present page directory entries.
 *
 *  @return Void.
 */
void vm_clear() {
    unsigned va = USER_MEM_START;
    while (va >= USER_MEM_START) {
        pde_t pde = GET_PDE(GET_PD(), va);
        if (!GET_PRESENT(pde)) {
            va += (1 << PD_SHIFT);
            continue;
         }

        pte_t pte = GET_PTE(pde, va);
        if (!GET_PRESENT(pte)) {
            va += (1 << PT_SHIFT);
            continue;
        }

        vm_remove_pte((void *)va);

        va += PAGE_SIZE;
    }

    set_cr3(get_cr3());
}

/** @brief Removes a page directory.
 *
 *  Removes a page directory, all of its page tables, and its mapped physical
 *  frames.
 *
 *  @return Void.
 */
void vm_destroy(pd_t pd) {
    vm_clear();

    unsigned va;
    for(va = KERNEL_MEM_START; va < USER_MEM_START; va += PAGE_SIZE) {
        vm_remove_pte((void *)va);
    }

    sfree(pd, PAGE_SIZE);
}

/** @brief Allocated memory starting at base and extending for len bytes.
 *
 *  @param base The base of the memory region to allocate.
 *  @param len The number of bytes to allocate.
 *  @return 0 on success, negative error code otherwise.
 */
int new_pages(void *base, int len)
{
    if ((unsigned)base > (unsigned)base + len - 1) {
        return -1;
    }

    if (((unsigned)base % PAGE_SIZE) != 0) {
        return -2;
    }

    if ((len % PAGE_SIZE) != 0) {
        return -3;
    }

    if ((unsigned)base < USER_MEM_START)
        return -4;

    if ((unsigned)base > -(unsigned)len)
        return -5;

    unsigned va;
    for (va = (unsigned)base; va < (unsigned)base + len - 1; va += PAGE_SIZE) {
        if (vm_is_present((void *)va)) {
            return -3;
        }
    }

    for (va = (unsigned)base; va < (unsigned)base + len - 1; va += PAGE_SIZE) {
        vm_new_pte(GET_PD(), (void *)va, get_frame(), USER_FLAGS);
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
    if (((unsigned)base % PAGE_SIZE) != 0) {
        return -1;
    }

    int len;
    if (hashtable_get(&alloc_pages, PAGE_NUM(base), (void**)&len) < 0) {
        return -2;
    }

    hashtable_remove(&alloc_pages, PAGE_NUM(base));

    unsigned va;
    for (va = (unsigned)base; va < (unsigned)base + len - 1; va += PAGE_SIZE) {
        vm_remove_pte((void *)va);
    }

    return 0;
}
