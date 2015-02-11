/** @file linklist.h
 *  @brief This file defines the type and function prototypes for linked lists.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _LINKLIST_H
#define _LINKLIST_H

typedef struct listnode {
    void *data;
    struct listnode *next;
} listnode_t;

typedef struct linklist {
    listnode_t *head;
    listnode_t *tail;
} linklist_t;

/** @brief Initializes a list to be empty.
 *
 *  @param list List.
 *  @return Void.
 */
void linklist_init(linklist_t *list);

/** @brief Adds a node to the head of a list.
 *
 *  @param list List.
 *  @param node Node.
 *  @return Void.
 */
void linklist_add_head(linklist_t *list, listnode_t *node);

/** @brief Adds a node to the tail of a list.
 *
 *  @param list List.
 *  @param node Node.
 *  @return Void.
 */
void linklist_add_tail(linklist_t *list, listnode_t *node);

/** @brief Removes the node at the head of a list.
 *
 *  @param list List.
 *  @return The removed node or NULL of the list is empty.
 */
listnode_t *linklist_remove_head(linklist_t *list);

#endif /* _LINKLIST_H */
