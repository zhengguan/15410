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
#include <proc.h>
#include <assert.h>

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
int hashtable_add(hashtable_t *table, int key, void *data)
{
    if (table == NULL) {
        return -1;
    }

    hashnode_t *addnode = (hashnode_t *)malloc(sizeof(hashnode_t));
    if (addnode == NULL) {
        return -2;
    }
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

    return 0;
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

/**
 * @brief Destroys a hashtable.
 * Everything currently in the hashtable is lost.
 *
 * @param ht The table to destroy.
 */
void hashtable_destroy(hashtable_t *ht)
{
    int i;
    for (i = 0; i < ht->size; i++) {
        hashnode_t *node = ht->nodes[i];
        while (node != NULL) {
            hashnode_t *next = node->next;
            free(node);
            node = next;
        }
    }
    free(ht->nodes);
    ht->nodes = NULL;
    ht->size = 0;
}

/**
 * @brief Copies all elements in the hashtable.
 * @param oldht The hashtable to copy from.
 * @param newht The hashtable to copy to.
 *
 * @return Zero on success or a negative error code on failure.
 */
int hashtable_copy(hashtable_t *oldht, hashtable_t *newht)
{
    assert(oldht != NULL && newht != NULL);
    assert(oldht != newht);
    assert(newht->size == oldht->size);
    int i;
    for (i = 0; i < oldht->size; i++) {
        hashnode_t *oldnode = oldht->nodes[i];
        if (oldnode == NULL)
            continue;
        else {
            hashnode_t *newnode = malloc(sizeof(hashnode_t));
            if (newnode == NULL) {
                hashtable_destroy(newht);
                return -1;
            }
            newnode->key = oldht->nodes[i]->key;
            newnode->data = oldht->nodes[i]->data;
            newht->nodes[i] = newnode;

            while (oldnode->next != NULL) {
                newnode->next = malloc(sizeof(hashnode_t));
                if (newnode->next == NULL) {
                    hashtable_destroy(newht);
                    return -1;
                }
                newnode = newnode->next;
                oldnode = oldnode->next;
                newnode->key = oldnode->key;
                newnode->data = oldnode->data;
            }
            newnode->next = NULL;
        }
    }
    return 0;
}
