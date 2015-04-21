/** @file linklist.h
 *  @brief This file defines the type and function prototypes for linked lists.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _FREE_PAGE_LINKLIST_H
#define _FREE_PAGE_LINKLIST_H

/* linked list functions */
void free_page_add(unsigned pa);
unsigned free_page_remove();


#endif /* _FREE_PAGE_LINKLIST_H */
