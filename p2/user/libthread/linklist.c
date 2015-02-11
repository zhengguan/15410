/** @file linklist.c
 *  @brief An implementation of linked lists.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include <linklist.h>
#include <stdlib.h>

void linklist_init(linklist_t *list) {
    list->head = NULL;
    list->tail = NULL;
}

void linklist_add_head(linklist_t *list, listnode_t *node) {
    if (list->head == NULL) {
        list->tail = node;
    } else {
        node->next = list->head;
    }
    list->head = node;
}

void linklist_add_tail(linklist_t *list, listnode_t *node) {
    if (list->tail == NULL) {
        list->head = node;
    } else {
        list->tail->next = node;
    }
    list->tail = node;
}

listnode_t *linklist_remove_head(linklist_t *list) {
    listnode_t *node = list->head;
    
    if (node != NULL) {
        list->head = node->next;
    } 
    
    if (list->head == NULL) {
        list->tail = NULL;
    }
    
    return node;
}
