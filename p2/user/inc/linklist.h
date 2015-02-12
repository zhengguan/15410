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

/* linked list functions */
int linklist_init(linklist_t *list);
void linklist_add_head(linklist_t *list, listnode_t *node);
void linklist_add_tail(linklist_t *list, listnode_t *node);
listnode_t *linklist_remove_head(linklist_t *list);
listnode_t *linklist_remove_all(linklist_t *list);

#endif /* _LINKLIST_H */
