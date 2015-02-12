/** @file linklist.c
 *  @brief An implementation of linked lists.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include <linklist.h>
#include <stdlib.h>

/** @brief Initializes a list to be empty.
 *
 *  @param list List.
 *  @return 0 on success, negative number on failure.
 */
int linklist_init(linklist_t *list) {
    if (list == NULL) {
        return -1;
    }

    list->head = NULL;
    list->tail = NULL;
    
    return 0;
}

/** @brief Adds a node to the head of a list.
 *
 *  @param list List.
 *  @param node Node.
 *  @return Void.
 */
void linklist_add_head(linklist_t *list, listnode_t *node) {
    if (list == NULL || node == NULL) {
        return;
    }
    
    if (list->head == NULL) {
        list->tail = node;
    } else {
        node->next = list->head;
    }
    list->head = node;
}

/** @brief Adds a node to the tail of a list.
 *
 *  @param list List.
 *  @param node Node.
 *  @return Void.
 */
void linklist_add_tail(linklist_t *list, listnode_t *node) {
    if (list == NULL || node == NULL) {
        return;
    }
    
    if (list->tail == NULL) {
        list->head = node;
    } else {
        list->tail->next = node;
    }
    list->tail = node;
}

/** @brief Removes the node at the head of a list.
 *
 *  @param list List.
 *  @return The removed node or NULL of the list is empty.
 */
listnode_t *linklist_remove_head(linklist_t *list) {
    if (list == NULL) {
        return NULL;
    }
    
    listnode_t *node = list->head;
    
    if (node != NULL) {
        list->head = node->next;
    } 
    
    if (list->head == NULL) {
        list->tail = NULL;
    }
    
    return node;
}

/** @brief Removes all node from a list.
 *
 *  @param list List.
 *  @return The head of the removed nodes or NULL of the list is empty.
 */
listnode_t *linklist_remove_all(linklist_t *list) {
    if (list == NULL) {
        return NULL;
    }
    
    listnode_t *node = linklist_remove_head(list);
    
    while (list->head != NULL) {
        linklist_remove_head(list);
    }
    
    return node;
}
