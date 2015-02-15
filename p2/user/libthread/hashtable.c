/** @file hashtable.c
 *  @brief An implementation of hash tables.
 *
 *  Simple integer modulo for a hash function.
 *  Uses linked lists for collision resolution.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#include <hashtable.h>
#include <stdlib.h>
#include <string.h>
#include <simics.h>

/**
 * @brief Determines the index in the hashtable from the key.
 * @param table The hashtable
 * @param key The key
 * @return The index
 */
static inline int hashtable_idx(hashtable_t *table, int key) {
    return key % table->size;
}

/** @brief Initializes a hash table to be empty.
 *
 *  @param table Hash table.
 *  @return Pointer to the hash table, NULL on failure.
 */
hashtable_t *hashtable_init(int size) {
    hashtable_t *table = (hashtable_t *)malloc(sizeof(hashtable_t) + size*sizeof(hashnode_t));
    if (table == NULL)
        return NULL;
    table->size = size;
    memset(table->nodes, 0, size*sizeof(hashnode_t));

    return table;
}

/** @brief Adds a node to a hash table.
 *
 *  @param table Hash table.
 *  @param key Key.
 *  @param data Data.
 *  @return Void.
 */
void hashtable_add(hashtable_t *table, int key, void *data) {
    if (table == NULL) {
        return;
    }

    lprintf("adding %d", key);

    hashnode_t *addnode = (hashnode_t *)malloc(sizeof(hashnode_t *));
    addnode->key = key;
    addnode->data = data;
    addnode->next = NULL;

    int idx = hashtable_idx(table, key);

    hashnode_t *node = table->nodes[idx];

    if (node == NULL) {
        table->nodes[idx] = addnode;
    } else {
        while (node->next != NULL) {
            node = node->next;
        }
        node->next = addnode;
    }
}

/** @brief Gets data from a hash table.
 *
 *  @param table Hash table.
 *  @param key Key.
 *  @return The data, or NULL if key does not exist in the hash table.
 */
void *hashtable_get(hashtable_t *table, int key) {
    if (table == NULL) {
        return NULL;
    }

    lprintf("getting %d", key);


    int idx = hashtable_idx(table, key);

    hashnode_t *node = table->nodes[idx];

    while (node != NULL) {
        if (node->key == key) {
            return node->data;
        }
        node = node->next;
    }

    lprintf("not found: %d", key);

    return NULL;
}

/** @brief Removes a node from a hash table.
 *
 *  @param table Hash table.
 *  @param key Key.
 *  @return Void.
 */
void hashtable_remove(hashtable_t *table, int key) {
    if (table == NULL) {
        return;
    }

    int idx = hashtable_idx(table, key);

    hashnode_t *node = table->nodes[idx];

    if (node != NULL) {
        if (node->key == key) {
            table->nodes[idx] = node->next;
            free(node);
        } else {
            while (node->next != NULL) {
                if (node->next->key == key) {
                    hashnode_t *oldnode = node->next;
                    node->next = node->next->next;
                    free(oldnode);
                }
                node = node->next;
            }
        }
    }
}
