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
 *  @param list The list.
 *  @return 0 on success, negative error code otherwise.
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
 *  @param list The list.
 *  @param data The data.
 *  @return Void.
 */
void linklist_add_head(linklist_t *list, void *data) {
    if (list == NULL) {
        return;
    }

    listnode_t *node = malloc(sizeof(listnode_t));
    if (node == NULL) {
        return;
    }
    
    node->data = data;
    node->next = list->head;
    list->head = node;

    if (node->next == NULL) {
        list->tail = node;
    }
}

/** @brief Adds a node to the tail of a list.
 *
 *  @param list The list.
 *  @param data Data.
 *  @return Void.
 */
void linklist_add_tail(linklist_t *list, void *data) {
    if (list == NULL) {
        return;
    }

    listnode_t *node = malloc(sizeof(listnode_t));
    if (node == NULL) {
        return;
    }
    
    node->data = data;
    node->next = NULL;

    if (list->tail == NULL) {
        list->head = node;
    } else {
        list->tail->next = node;
    }
    list->tail = node;
}

/** @brief Removes the node at the head of a list.
 *
 *  @param list The list.
 *  @param data A location in memory to store the remove data.
 *  @return 0 on success, negative error code otherwise.
 */
int linklist_remove_head(linklist_t *list, void **data) {
    if (list == NULL) {
        return -1;
    }

    listnode_t *node = list->head;

    if (node == NULL) {
        return -2;
    }

    list->head = node->next;

    if (list->head == NULL) {
        list->tail = NULL;
    }

    if (data != NULL) {
        *data = node->data;
    }

    free(node);

    return 0;
}

/** @brief Removes the first instance of data in the list.
 *
 *  @param list The list.
 *  @param data The data of the item to remove.
 *  @return Void.
 */
void linklist_remove(linklist_t *list, void *data) {
    if (list == NULL || list->head == NULL) {
        return;
    }
    
    listnode_t *node = list->head;
    while (node->next != NULL) {
        if (node->next->data == data) {
            listnode_t *tmp = node->next;
            node->next = tmp->next;
            free(tmp);
            break;
        }
        node = node->next;
    }
    
    return;
}

/** @brief Removes all nodes from a list.
 *
 *  @param list The list.
 *  @return 0 on success, negative error code otherwise.
 */
int linklist_remove_all(linklist_t *list) {
    if (list == NULL) {
        return -1;
    }

    while (linklist_remove_head(list, NULL) == 0) {
    }

    return 0;
}

/** @brief Checks whether a list contains data.
 *  
 *  @param list The list.
 *  @param data The data of the item to remove.
 *  @return True if the list contains the data, false otherwise.
 */
bool linklist_contains(linklist_t *list, void *data) {
    if (list == NULL || list->head == NULL) {
        return false;
    }
    
    listnode_t *node = list->head;
    while (node->next != NULL) {
        if (node->next->data == data) {
            return true;
        }
        node = node->next;
    }
    
    return false;
}

/** @brief Moves all nodes in a linked list to a new linked list
 *
 *  @param list The list.
 *  @param oldlist The list to move the nodes from.
 *  @param newlist The list to move the nodes to.
 *  @return 0 on success, negative error code otherwise.
 */
int linklist_move(linklist_t *oldlist, linklist_t* newlist) {
    if (oldlist == NULL || newlist == NULL) {
        return -1;
    }

    newlist->head = oldlist->head;
    newlist->tail = oldlist->tail;

    return linklist_init(oldlist);
}

/**
 * @brief Determines if a linked list is empty.
 *
 * @param list The list
 * @return True if the list is empty or NULL, false otherwise.
 */
int linklist_empty(linklist_t *list)
{
    return (list == NULL) || (list->head == NULL);
}
