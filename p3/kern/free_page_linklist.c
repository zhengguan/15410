/** @file linklist.c
 *  @brief An implementation of linked lists.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <stdlib.h>
#include <vm.h>
#include <simics.h>

static unsigned ll_head = 0;

/**
 * @brief Adds a free page to a list of free pages.
 * @details Stores the next physical page in the physical pages.
 *
 * @param pa The physical address.
 */
void free_page_add(unsigned pa)
{
    vm_phys_write(pa, ll_head);
    ll_head = pa;
}

/**
 * @brief Removes a free page from the list of free pages.
 * @return The physical address that we removed or 0 if the list is empty.
 */
unsigned free_page_remove()
{
    if (ll_head == 0) {
        return 0;
    }
    unsigned pa = ll_head;
    ll_head = vm_phys_read(pa);
    return pa;
}