#ifndef HASHMAP_H
#define HASHMAP_H

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>

enum hashmapresult
{
    HASHMAPRESULT_SUCCESS,
    HASHMAPRESULT_FAILURE_BAD_INITIALIZATION,
    HASHMAPRESULT_FAILURE_ALLOCATION,
    HASHMAPRESULT_FAILURE_OVERFLOW,
    HASHMAPRESULT_FAILURE_KEY_EXISTS,
    HASHMAPRESULT_FAILURE_KEY_NOTEXISTS,
};

/**
 * A node for the linked list of a hash map bin.
 */
struct hashmapbinnode
{
    size_t key;
    char *value;
    struct hashmapbinnode *next;
};

/**
 * A hash map with chaining. Each bin contains a linked list of nodes.
 */
struct hashmap
{
    struct hashmapbinnode **bins;
    size_t bins_count;
    size_t entries_count;
};

/**
 * NOTE: The hash map has to be zero-initialized.
 */
enum hashmapresult hashmap_initialize(struct hashmap *hm);
void hashmap_delete(struct hashmap *hm);
/**
 * Puts a new key value pair into the hash map. Increases the amount of bins
 * automatically if a certain load was reached.
 *
 * NOTE: The value must not be `NULL`.
 * NOTE: The increase of bins is not reverted in case the value could not be added.
 */
enum hashmapresult hashmap_put(struct hashmap *hm, size_t key, const char *value);
/**
 * Return values:
 * - `true`: Key does exist and is returned through `out_value` if it's not NULL.
 * - `false`: Key does not exist.
 */
bool hashmap_tryget(const struct hashmap *hm, size_t key, const char **out_value);
enum hashmapresult hashmap_remove(struct hashmap *hm, size_t key);
void hashmap_fprint(FILE *file, const struct hashmap *hm);

#endif /* HASHMAP_H */