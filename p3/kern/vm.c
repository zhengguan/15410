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
#include <hashtable.h>
#include <simics.h>
#include <string.h>
#include <kern_common.h>
#include <mutex.h>
#include <proc.h>
#include <memlock.h>
#include <assert.h>
#include <free_page_linklist.h>
#include <asm_common.h>

#define PHYS_VA 0xC0001000

#define KERNEL_MEM_START 0x00000000

#define FLAG_MASK (~PAGE_MASK & ~1)
#define GET_PRESENT(PTE) ((PTE) & PTE_PRESENT)
#define GET_SU(PTE) ((PTE) & PTE_SU)
#define GET_FLAGS(PTE) ((PTE) & FLAG_MASK)

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
#define FRAME_NUM(ADDR) (LOOKUP_PA(ADDR) >> PT_SHIFT)

int vm_new_pde(pde_t *pde, pt_t pt, unsigned flags);
int vm_new_pt(pde_t *pde, unsigned flags);
int vm_new_pte(pd_t pd, void *va, unsigned pa, unsigned flags);
void vm_remove_pde(pde_t *pde);
void vm_remove_pt(pde_t *pde);
void vm_remove_pte(pd_t pd, void *va);

unsigned next_frame = USER_MEM_START;
mutex_t free_frames_mutex;

/** @brief Gets a free physical frame.
 *
 *  Returns a previously allocated free physical frame if one is available.
 *  Otherwise it gets a new free physical frame.
 *
 *  @param frame Memory address to write the physical address of the frame.
 *  @return 0 on success, negative error code otherwise.
 */
static int get_frame(unsigned *frame)
{
    mutex_lock(&free_frames_mutex);
    if ( (void*)(*frame = free_page_remove()) == NULL) {
        *frame = next_frame;
        if (*frame > machine_phys_frames() * PAGE_SIZE) {
            mutex_unlock(&free_frames_mutex);
            return -1;
        }
        next_frame += PAGE_SIZE;
    }
    mutex_unlock(&free_frames_mutex);

    return 0;
}

/**
 * @brief Set PHYS_VA to point to a particular physical frame.
 *
 * @param pa The physical frame address.
 */
static void vm_set_phys_pte(unsigned pa)
{
    if (pa == 0xC0000000)
        MAGIC_BREAK;
    assert(vm_new_pte(GET_PD(), (void *)PHYS_VA, pa, PTE_PRESENT | PTE_RW) == 0);
    flush_tlb_entry((void*)PHYS_VA);
}

/** @brief Checks whether the page table referenced by a page directory entry
 *  is empty.
 *
 *  @param pde The page directory entry.
 *  @return True if empty, false otherwise.
 */
static bool is_pt_empty(pde_t *pde)
{
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
    if (mutex_init(&free_frames_mutex) < 0) {
        return -3;
    }

    pd_t pd;
    if (vm_new_pd(&pd) < 0) {
        return -5;
    }
    set_cr3((unsigned)pd);

    //Enable paging
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

    memset(pd, 0, PAGE_SIZE);

    if (vm_new_pte(pd, (void*)PHYS_VA, 0, KERNEL_FLAGS) < 0) {
        sfree(pd, PAGE_SIZE);
        return -3;
    }

    unsigned pa;
    for (pa = KERNEL_MEM_START; pa < USER_MEM_START; pa += PAGE_SIZE) {
        if (vm_new_pte(pd, (void *)pa, pa, KERNEL_FLAGS) < 0) {
            vm_destroy(pd);
            return -2;
        }
    }

    vm_set_phys_pte(0);

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
    *pde = (GET_PA(pt) | PTE_PRESENT | flags | PTE_SU);

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

    memset(pt, 0, PAGE_SIZE);

    if (vm_new_pde(pde, pt, flags) < 0) {
        free(pt);
        return -2;
    }

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
void vm_remove_pte(pd_t pd, void *va) {
    if (!vm_check_flags(pd, va, PTE_PRESENT, 0)) {
        return;
    }

    pde_t *pde = pd + GET_PD_IDX(va);

    pte_t *pte =  GET_PT(*pde) + GET_PT_IDX(va);
    *pte &= ~PTE_PRESENT;

    unsigned pa = GET_PA(*pte);
    if (pa >= USER_MEM_START) {
        mutex_lock(&free_frames_mutex);
        free_page_add((unsigned)(*pte & PAGE_MASK));
        mutex_unlock(&free_frames_mutex);
    }

    if (pd == GET_PD())
        flush_tlb_entry(va);

    if (is_pt_empty(pde)) {
        vm_remove_pt(pde);
    }
}

/** @brief Copies the virtual address space into new page directory.
 *
 *  Create a new page directory and iterate over the address space and copy all
 *  present page table entries into the new page directory.
 *
 *  @return 0 on success, negative error code otherwise.
 */
int vm_copy(pd_t *new_pd, hashtable_t *new_alloc_pages)
{
    assert(getpcb()->num_threads == 1);
    pd_t old_pd = GET_PD();
    if (vm_new_pd(new_pd) < 0) {
        return -1;
    }

    char *va = (char*)USER_MEM_START;
    while ((unsigned)va >= USER_MEM_START) { //goes to end of memory
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

        unsigned frame;
        if (get_frame(&frame) < 0) {
            vm_destroy(*new_pd);
            return -3;
        }

        if (vm_new_pte(*new_pd, va, frame, GET_FLAGS(pte)) < 0) {
            mutex_lock(&free_frames_mutex);
            free_page_add(frame);
            mutex_unlock(&free_frames_mutex);
            vm_destroy(*new_pd);
            return -4;
        }

        vm_set_phys_pte(frame);
        memcpy((char*)PHYS_VA, va, PAGE_SIZE);

        va += PAGE_SIZE;
    }

    if (hashtable_copy(&getpcb()->alloc_pages, new_alloc_pages) < 0) {
        vm_destroy(new_pd);
        return -5;
    }

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
    assert(getpcb()->num_threads == 1);
    unsigned va = USER_MEM_START;
    while (va >= USER_MEM_START) {
        pde_t pde = GET_PDE(GET_PD(), va);
        if (!GET_PRESENT(pde)) {
            va += (1 << PD_SHIFT);
            continue;
         }

        pte_t pte = GET_PTE(pde, va);
        if (!GET_PRESENT(pte) || !GET_SU(pte)) {
            va += (1 << PT_SHIFT);
            continue;
        }
        vm_remove_pte(GET_PD(), (void *)va);
        //no need to lock. Single threaded
        hashtable_remove(&getpcb()->alloc_pages, va, NULL);

        va += PAGE_SIZE;
    }
}

/** @brief Removes a page directory.
 *
 *  Removes a page directory. Must call vm_clear() before this call.
 *
 *  @return Void.
 */
void vm_destroy(pd_t pd) {
    assert(pd != GET_PD());
    unsigned va;
    for(va = KERNEL_MEM_START; va < USER_MEM_START; va += PAGE_SIZE) {
        vm_remove_pte(pd, (void *)va);
    }

    sfree(pd, PAGE_SIZE);
}

/** @brief Sets a virtual address to be read-only.
 *
 *  @param va The virtual address.
 *  @return Void.
 */
void vm_read_only(void *va) {
    pde_t pde = GET_PDE(GET_PD(), va);
    pte_t *pte = GET_PT(pde) + GET_PT_IDX(va);
    *pte &= ~PTE_RW;
}

/** @brief Sets a virtual address to be read-write.
 *
 *  @param va The virtual address.
 *  @return Void.
 */
void vm_read_write(void *va) {
    pde_t pde = GET_PDE(GET_PD(), va);
    pte_t *pte = GET_PT(pde) + GET_PT_IDX(va);
    *pte |= PTE_RW;
}

/** @brief Sets a virtual address to be supervisor.
 *
 *  @param va The virtual address.
 *  @return Void.
 */
void vm_super(void *va) {
    pde_t pde = GET_PDE(GET_PD(), va);
    pte_t *pte = GET_PT(pde) + GET_PT_IDX(va);
    *pte &= ~PTE_SU;
}

/** @brief Sets a virtual address to be user.
 *
 *  @param va The virtual address.
 *  @return Void.
 */
void vm_user(void *va) {
    pde_t pde = GET_PDE(GET_PD(), va);
    pte_t *pte = GET_PT(pde) + GET_PT_IDX(va);
    *pte |= PTE_SU;
}

/** @brief Checks if flags are set for a virtual memory address.
 *
 * @param va The virtual address of which to check the flags.
 * @param flags The flags.
 * @return True if all given flags are set and false otherwise.
 */
bool vm_check_flags(pd_t pd, void *va, unsigned reqflags, unsigned badflags)
{
    va = (void*)ROUND_DOWN_PAGE(va);
    pde_t pde = GET_PDE(pd, va);
    if (GET_PRESENT(pde)) {
        pte_t pte = GET_PTE(pde, va);
        return ((pte & reqflags) == reqflags) && !(pte & badflags);
    }

    return !(reqflags & PTE_PRESENT);
}

/**
 * @brief Checks the flags of a virtual address.
 * @details Ensures the flags of the page table entry
 * are at least the given flags.
 *
 * @param base The base of the virtual memory of which to check the flags.
 * @param len The length of memory to check.
 * @param flags The flags.
 * @return True if all given flags are set and false otherwise.
 */
bool vm_check_flags_len(pd_t pd, void *base, int len, unsigned reqflags, unsigned badflags)
{
    unsigned va = (unsigned)base;

    if (va > va + len - 1) {
        return false;
    }

    for (; va < (unsigned)base + len - 1; va += PAGE_SIZE) {
        if (!vm_check_flags(pd, (void*)va, reqflags, badflags)) {
            return false;
        }
    }

    return true;
}

/**
 * @brief Locks an address and checks its flags.
 * @details Once locked, the page cannot be removed until it is unlocked.
 * Multiple readers can hold the lock at once.
 * If the address is not of user privilege level or present
 * then the page is not locked.
 *
 * @param va The virtual address to lock.
 * @return A boolean indicating whether the address is present and
 * has user privilege level.
 */
bool vm_lock(void *va, unsigned reqflags, unsigned badflags, unsigned access) {
    va = (void*)ROUND_DOWN_PAGE((unsigned)va);
    mutex_lock(&getpcb()->locks.vm_lock);

    bool valid = vm_check_flags(GET_PD(), va, reqflags, badflags);
    if (valid) {
        memlock_lock(&getpcb()->locks.memlock, va, access);
    }
    mutex_unlock(&getpcb()->locks.vm_lock);
    return valid;
}

/**
 * @brief Locks a length of virtual memory and checks its flags.
 * @details Once locked, no pages can be removed until unlocked.
 * Multiple readers can hold the lock at once.
 * If any of the addresses are not present or not of user privilege level,
 * then no page is locked.
 *
 * @param base The lowest virtual address to lock.
 * @param len The length of memory to lock.
 * @return A boolean indicating whether the whole memory length is present
 * and has user privilege level.
 */
 bool vm_lock_len(void *base, int len, unsigned reqflags, unsigned badflags, unsigned access) {
    mutex_lock(&getpcb()->locks.vm_lock);

    bool valid = vm_check_flags_len(GET_PD(), base, len, reqflags, badflags);
    if (valid) {
        unsigned va;
        for (va = ROUND_DOWN_PAGE((unsigned)base);  va < (unsigned)base + len - 1; va += PAGE_SIZE) {
            memlock_lock(&getpcb()->locks.memlock, (void *)va, access);
        }
    }
    mutex_unlock(&getpcb()->locks.vm_lock);

    return valid;
}

/**
 * @brief Locks a string in memory and checks its privilege level and length.
 * @details Once locked, no pages can be removed until unlocked.
 * Multiple readers can hold the lock at once.
 * If any of the addresses are not present or not of user privilege level
 * before a nul terminator is found, no memory is locked.
 *
 * @param str The string to lock.
 * @return The length of the string or a negative error code if the string
 * is not present, or not of user privilege level.
 */
int vm_lock_str(char *str, unsigned reqflags, unsigned badflags, unsigned access) {
    mutex_lock(&getpcb()->locks.vm_lock);
    int len = str_check(str, reqflags, badflags);
    if (len >= 0) {
        unsigned va;
        for (va = (unsigned)str;  va < (unsigned)str + len - 1; va += PAGE_SIZE) {
            memlock_lock(&getpcb()->locks.memlock, (void *)va, access);
        }
    }
    mutex_unlock(&getpcb()->locks.vm_lock);
    return len;
}

/** @brief Unlocks a virtual memory address.
 *
 *  @param va The virtual address.
 *  @return Void.
 */
void vm_unlock(void *va) {
        memlock_unlock(&getpcb()->locks.memlock, (void *)va);
}

/** @brief Unlocks a virtual memory region.
 *
 *  @param base The base virtual address.
 *  @param len The length.
 *  @return Void.
 */
void vm_unlock_len(void *base, int len) {
    unsigned va;
    for (va = (unsigned)base;  va < (unsigned)base + len - 1; va += PAGE_SIZE) {
        memlock_unlock(&getpcb()->locks.memlock, (void *)va);
    }
}

/** @brief Reads from a physical address.
 *
 *  @param pa The physical address.
 *  @return The value.
 */
unsigned vm_phys_read(unsigned pa) {
    vm_set_phys_pte(pa);
    return *(unsigned *)(PHYS_VA + (pa & (~PAGE_MASK)));
}

/** @brief Writes to a physical address.
 *
 *  @param pa The physical address.
 *  @param val The value.
 *  @return Void.
 */
void vm_phys_write(unsigned pa, unsigned val) {
    vm_set_phys_pte(pa);
    *(unsigned *)(PHYS_VA + (pa & (~PAGE_MASK))) = val;
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

    if ((unsigned)base < USER_MEM_START) {
        return -4;
    }

    if ((unsigned)len/PAGE_SIZE >
        machine_phys_frames() - USER_MEM_START/PAGE_SIZE)
        return -5;


    //TODO: maybe fix. Locks even if gigantic number and we don't have
    //that much space.
    if (!vm_lock_len(base, len, 0, PTE_PRESENT, MEMLOCK_MODIFY)) {
        return -5;
    }

    //Try to add to hashtable of stored lengths
    mutex_lock(&getpcb()->locks.alloc_pages_lock);
    if (hashtable_add(&getpcb()->alloc_pages, (int)base, (void *)len) < 0) {
        mutex_unlock(&getpcb()->locks.alloc_pages_lock);
        vm_unlock_len(base, len);
        return -6;
    }

    mutex_unlock(&getpcb()->locks.alloc_pages_lock);

    //Add the new pages
    void* va;
    for (va = base; va < base + len - 1; va += PAGE_SIZE) {
        unsigned frame;
        //disable user access for now
        if (get_frame(&frame) < 0 ||
            vm_new_pte(GET_PD(), va, frame, USER_FLAGS_RW & ~PTE_SU) < 0) {
            //if we fail, we have to remove pages
            for (va -= PAGE_SIZE; va >= base; va -= PAGE_SIZE) {
                vm_remove_pte(GET_PD(), va);
            }
            //remove from hashtable
            mutex_lock(&getpcb()->locks.alloc_pages_lock);
            hashtable_remove(&getpcb()->alloc_pages, (int)base, NULL);
            mutex_unlock(&getpcb()->locks.alloc_pages_lock);

            vm_unlock_len((void *)base, len);
            return -6;
        }
        assert(vm_check_flags(GET_PD(), va, USER_FLAGS_RW & ~PTE_SU, PTE_SU));
        //set to 0
        memset(va, 0, PAGE_SIZE);
        //enable user access
        vm_user(va);
    }

    vm_unlock_len(base, len);

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

    //get the length from the table
    mutex_lock(&getpcb()->locks.alloc_pages_lock);
    int len;
    if (hashtable_remove(&getpcb()->alloc_pages, (int)base, (void**)&len) < 0) {
        mutex_unlock(&getpcb()->locks.alloc_pages_lock);
        return -2;
    }
    mutex_unlock(&getpcb()->locks.alloc_pages_lock);

    //Lock the pages. They should be present
    assert(vm_lock_len(base, len, PTE_PRESENT, 0, MEMLOCK_MODIFY));

    unsigned va;
    for (va = (unsigned)base; va < (unsigned)base + len - 1; va += PAGE_SIZE) {
        vm_remove_pte(GET_PD(), (void *)va);
    }

    vm_unlock_len(base, len);
    return 0;
}
