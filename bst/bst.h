#ifndef BST_H
#define BST_H

#include <stdio.h>
#include <stdbool.h>

enum bstresult
{
    BSTRESULT_SUCCESS,
    BSTRESULT_FAILURE_ALLOCATION,
    BSTRESULT_FAILURE_VALUE_EXISTS,
};

struct bstnode
{
    struct bstnode *left;
    struct bstnode *right;
    int value;
};

struct bst
{
    struct bstnode *root;
};

void bst_init(struct bst *bst);
void bst_del(struct bst *bst);
/**
 * NOTE: If the BST already contains the value, we don't add it again.
 */
enum bstresult bst_add(struct bst *bst, int value);
/**
 * Returns `true` if the value is inside the BST, else `false`.
 */
bool bst_in(const struct bst *bst, int value);
/**
 * Removes an existing value from the BST and returns `true`. If the value
 * does not exist `false` is returned.
 *
 * NOTE: If the removed node has two successors, the in-order successor is chosen
 * as the replacement.
 */
bool bst_rmv(struct bst *bst, int value);
void bst_fprint(FILE *file, const struct bst *bst);

#endif /* BST_H */