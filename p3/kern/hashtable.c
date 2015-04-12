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

struct hashnode {
    int key;
    void *data;
    struct hashnode *next;
};

/**
 * @brief Determines the index in the hash table from the key.
 *
 * @param table The hash table.
 * @param key The key.
 * @return The index.
 */
static inline int hashtable_idx(hashtable_t *table, int key)
{
    unsigned h = key;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;
    return h % table->size;
}

/** @brief Initializes a hash table to be empty.
 *
 *  @param size The hash table size.
 *  @return 0 on success, negative error code otherwise.
 */
int hashtable_init(hashtable_t *table, int size)
{
    if (table == NULL) {
        return -1;
    }

    table->nodes = malloc(size*sizeof(hashnode_t));
    if (table->nodes == NULL) {
        return -2;
    }

    table->size = size;
    memset(table->nodes, 0, size*sizeof(hashnode_t));

    return 0;
}

/** @brief Adds a node to a hash table.
 *
 *  @param table The hash table.
 *  @param key The key.
 *  @param data The data.
 *  @return Void.
 */
void hashtable_add(hashtable_t *table, int key, void *data)
{
    if (table == NULL) {
        return;
    }

    hashnode_t *addnode = (hashnode_t *)malloc(sizeof(hashnode_t));
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
 *  @param table The hash table.
 *  @param key The key.
 *  @param data A location in memory to store the gotten data.
 *  @return 0 on success, negative error code otherwise.
 */
int hashtable_get(hashtable_t *table, int key, void **data)
{
    if (table == NULL) {
        return -1;
    }

    int idx = hashtable_idx(table, key);

    hashnode_t *node = table->nodes[idx];

    while (node != NULL) {
        if (node->key == key) {
            if (data != NULL) {
                *data = node->data;
            }
            return 0;
        }
        node = node->next;
    }

    return -2;
}

/** @brief Removes a node from a hash table.
 *
 *  @param table The hash table.
 *  @param key The key.
 *  @param data A location in memory to store the removed data.
 *  @return 0 on success, negative error code otherwise.
 */
int hashtable_remove(hashtable_t *table, int key, void **data)
{
    if (table == NULL) {
        return -1;
    }

    int idx = hashtable_idx(table, key);

    hashnode_t *node = table->nodes[idx];

    if (node != NULL) {
        if (node->key == key) {
            table->nodes[idx] = node->next;
            if (data != NULL) {
                *data = node->data;
            }
            free(node);
            return 0;
        } else {
            while (node->next != NULL) {
                if (node->next->key == key) {
                    hashnode_t *oldnode = node->next;
                    node->next = node->next->next;
                    if (data != NULL) {
                        *data = oldnode->data;
                    }
                    free(oldnode);
                    return 0;
                }
                node = node->next;
            }
        }
    }

    return -2;
}

/** @brief Destroys a hashtable.
 *
 *  @param table The hashtable
 */
void hashtable_destroy(hashtable_t *table)
{
    if (table == NULL)
      return;

    free(table->nodes);
}