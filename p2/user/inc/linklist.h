/** @file linklist.h
 *  @brief This file defines the type and function prototypes for linked lists.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _LINKLIST_H
#define _LINKLIST_H

typedef struct listnode listnode_t;

typedef struct linklist {
    listnode_t *head;
    listnode_t *tail;
} linklist_t;

/* linked list functions */
linklist_t *linklist_init();
void linklist_add_head(linklist_t *list, void *data);
void linklist_add_tail(linklist_t *list, void *data);
int linklist_remove_head(linklist_t *list, void **data);
int linklist_remove_all(linklist_t *list, void **data);
int linklist_move(linklist_t *oldlist, linklist_t* newlist);
int linklist_empty(linklist_t *list);

#endif /* _LINKLIST_H */
