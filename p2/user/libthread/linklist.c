/** @file linklist.c
 *  @brief An implementation of linked lists.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <linklist.h>
#include <stdlib.h>

 struct listnode {
    void *data;
    struct listnode *next;
};

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
 *  @param data Data.
 *  @return Void.
 */
void linklist_add_head(linklist_t *list, void *data) {
    if (list == NULL) {
        return;
    }

    listnode_t *node = malloc(sizeof(listnode_t));
    node->data = data;
    node->next = list->head;
    list->head = node;

    //previously empty
    if (node->next == NULL) {
        list->tail = node;
    }

}

/** @brief Adds a node to the tail of a list.
 *
 *  @param list List.
 *  @param data Data.
 *  @return Void.
 */
void linklist_add_tail(linklist_t *list, void *data) {
    if (list == NULL) {
        return;
    }

    listnode_t *node = malloc(sizeof(listnode_t));
    node->data = data;
    node->next = NULL;

    //previously empty
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
 *  @param data A location in memory to store the data at the head.
 *  @return Evaluates to true if the list is nonempty and the head is
 *  removed and false otherwise.
 */
int linklist_remove_head(linklist_t *list, void **data) {
    if (list == NULL) {
        return 0;
    }

    listnode_t *node = list->head;

    if (node == NULL)
        return 0;

    list->head = node->next;

    if (list->head == NULL) {
        list->tail = NULL;
    }

    if (data != NULL)
        *data = node->data;

    free(node);

    return 1;
}

/** @brief Removes all nodes from a list.
 *
 *  @param list List.
 *  @param data A location in memory to store the data at the head.
 *  @return Evaluates to true if the list is nonempty and the head is
 *  removed and false otherwise.
 */
int linklist_remove_all(linklist_t *list, void **data) {
    if (list == NULL) {
        return 0;
    }

    if (!linklist_remove_head(list, data))
        return 0;

    //while there is still an element in the list
    while (linklist_remove_head(list, NULL)) { }

    return 1;
}

/** @brief Moves all nodes in a linked list to a new linked list
 *
 *  @param list List.
 *  @param oldlist The list to move the nodes from.
 *  @param newlist The list to move the nodes to.
 *  @return Evaluates to true iff the move was successful.
 */
int linklist_move(linklist_t *oldlist, linklist_t* newlist) {
    if (oldlist == NULL || newlist == NULL) {
        return 0;
    }

    newlist->head = oldlist->head;
    newlist->tail = oldlist->tail;

    oldlist->head = NULL;
    newlist->head = NULL;

    return 1;
}

/**
 * @brief Determines if a linked list is empty.
 * @param list List
 * @return Evaluates to true iff the linked list is empty or NULL
 */
int linklist_empty(linklist_t *list)
{
    return !list || !list->head;
}
