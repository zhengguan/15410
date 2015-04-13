/** @file hashtable.h
 *  @brief This file defines the type and function prototypes for hash tables.
 *
 *  @author Patrick Koenig (phkoenig)
 *  @author Jack Sorrell (jsorrell)
 *  @bug No known bugs.
 */

#ifndef _HASHTABLE_H
#define _HASHTABLE_H

typedef struct hashnode hashnode_t;

typedef struct hashtable {
    int size;
    hashnode_t **nodes;
} hashtable_t;

/* hash table functions */
int hashtable_init(hashtable_t *table, int size);
void hashtable_add(hashtable_t *table, int key, void *data);
int hashtable_get(hashtable_t *table, int key, void **data);
int hashtable_remove(hashtable_t *table, int key, void **data);

#endif /* _HASHTABLE_H */
