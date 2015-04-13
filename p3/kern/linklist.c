/** @file linklist.c
 *  @brief An implementation of linked lists.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <linklist.h>
#include <stdlib.h>
#include <assert.h>

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
void linklist_add_head(linklist_t *list, void *data, listnode_t *node) {
    if (list == NULL) {
        return;
    }

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
void linklist_add_tail(linklist_t *list, void *data, listnode_t *node) {
    if (list == NULL || node == NULL) {
        panic("Linklist passed bad values");
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

/** @brief Adds a node to a sorted list using the provided comparison function.
 *
 *  @param list The list.
 *  @param data Data.
 *  @param cmp Comparison function.
 *  @return Void.
 */
void linklist_add_sorted(linklist_t *list, void *data, data_cmp_t cmp, listnode_t *new_node) {
    if (list == NULL || new_node == NULL) {
        panic("Linklist passed bad values");
    }

    listnode_t *node = list->head;
    if (node == NULL || cmp(data, node->data) <= 0) {
        linklist_add_head(list, data, new_node);
        return;
    }

    while (node->next != NULL) {
        if (cmp(data, node->next->data) <= 0) {
            new_node->data = data;
            new_node->next = node->next;
            node->next = new_node;
            return;
        }
    }
    linklist_add_tail(list, data, new_node);
}

/** @brief Removes the node at the head of a list.
 *
 *  @param list The list.
 *  @param data A location in memory to store the removed data.
 *  @return 0 on success, negative error code otherwise.
 */
int linklist_remove_head(linklist_t *list, void **data, listnode_t **outnode)
{
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

    if (data) {
        *data = node->data;
    }
    if (outnode)
        *outnode = node;
    return 0;
}

/** @brief Looks at the node at the head of a list.
 *
 *  @param list The list.
 *  @param data A location in memory to store the removed data.
 *  @return 0 on success, negative error code otherwise.
 */
int linklist_peek_head(linklist_t *list, void **data) {
    if (list == NULL) {
        return -1;
    }

    listnode_t *node = list->head;

    if (node == NULL) {
        return -2;
    }

    if (data == NULL)
        return -3;

    *data = node->data;

    return 0;
}

/** @brief Removes the first instance of data in the list.
 *
 *  @param list The list.
 *  @param data The data of the item to remove.
 *  @param Equality test. Passed data is the second argument.
 *  @return 0 on success, negative error code otherwise.
 */
int linklist_remove(linklist_t *list, void *datakey, bool (*eq)(void*, void*), void **data, listnode_t **outnode)
{
    if (list == NULL || list->head == NULL) {
        return -1;
    }

    listnode_t *node = list->head;
    if (eq(node->data, datakey)) {
        return linklist_remove_head(list, data, outnode);
    }

    while (node->next != NULL) {
        if (eq(node->next->data, datakey)) {
            listnode_t *tmp = node->next;
            if (data)
                *data = tmp->data;
            if (outnode)
                *outnode = tmp;

            node->next = tmp->next;
            if (node->next == NULL) {
                list->tail = node;
            }
            assert(list->tail->next == NULL);
            return 0;
        }
        node = node->next;
    }

    return -2;
}

/** @brief Removes the first instance of data in the list.
 *
 *  @param list The list.
 *  @param data The data of the item to remove.
 *  @param Equality test. Passed data is the second argument.
 *  @return 0 on success, negative error code otherwise.
 */
int linklist_rotate_head(linklist_t *list, void **data) {
    if (list == NULL || list->head == NULL) {
        return -1;
    }

    listnode_t *node = list->head;
    if (node->next != NULL) {
        list->head = node->next;
        list->tail->next = node;
        list->tail = node;
        node->next = NULL;
    }

    if (data)
        *data = node->data;

    return 0;
}

int linklist_rotate_val(linklist_t *list, void *datakey, bool (*eq)(void*, void*), void **data) {
    if (list == NULL || list->head == NULL) {
        return -1;
    }

    listnode_t *node = list->head;
    if (eq(node->data, datakey))
        return linklist_rotate_head(list, data);

    while (node->next != NULL) {
        listnode_t *tmp = node->next;
        if (eq(tmp->data, datakey)) {

            if (tmp->next != NULL) {
                node->next = tmp->next;
                list->tail->next = tmp;
                list->tail = tmp;
                tmp->next = NULL;
            }

            if (data)
                *data = tmp->data;
            return 0;
        }
        node = node->next;
    }
    return -2;
}

/** @brief Checks whether a list contains data.
 *
 *  @param list The list.
 *  @param data The data of the item to remove.
 *  @param Equality test. Passed data is the second argument.
 *  @return True if the list contains the data, false otherwise.
 */
bool linklist_contains(linklist_t *list, void *data, bool (*eq)(void*, void*))
{
    if (list == NULL || list->head == NULL) {
        return false;
    }

    listnode_t *node = list->head;
    while (node->next != NULL) {
        if (eq(node->next->data, data)) {
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
int linklist_move(linklist_t *oldlist, linklist_t* newlist)
{
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
bool linklist_empty(linklist_t *list)
{
    return (list == NULL) || (list->head == NULL);
}