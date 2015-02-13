/** @file hashtable.c
 *  @brief An implementation of hash tables.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */
 
#include <hashtable.h>
#include <stdlib.h>

/** @brief Initializes a hash table to be empty.
 *
 *  @param table Hash table.
 *  @return Pointer to the hash table, NULL on failure.
 */
hashtable_t *hashtable_init(int size) {
    hashtable_t *table = (hashtable_t *)malloc(sizeof(hashtable_t));
    table->size = size;
    table->nodes = (hashnode_t **)calloc(size, sizeof(hashnode_t *));
    
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
    
    hashnode_t *addnode = (hashnode_t *)malloc(sizeof(hashnode_t *));
    addnode->key = key;
    addnode->data = data;
    addnode->next = NULL;
    
    hashnode_t *node = table->nodes[key % table->size];
    
    if (node == NULL) {
        table->nodes[key % table->size] = addnode;
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
    
    hashnode_t *node = table->nodes[key % table->size];
    
    while (node != NULL) {
        if (node->key == key) {
            return node->data;
        }
        node = node->next;
    }
    
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
    
    hashnode_t *node = table->nodes[key % table->size];
    
    if (node != NULL) {    
        if (node->key == key) {
            table->nodes[key % table->size] = node->next;
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
