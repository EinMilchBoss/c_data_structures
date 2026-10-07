#include <stdlib.h>
#include <assert.h>

#include "bst.h"

static void bstnode_init(struct bstnode *node, int value);
static void bst_fprint_rec(FILE *file, const struct bstnode *node);
static void bst_del_rec(struct bstnode *node);

static void bstnode_init(struct bstnode *node, int value)
{
    *node = (struct bstnode){
        .value = value,
    };
}

static void bst_del_rec(struct bstnode *node)
{
    if (!node)
        return;

    bst_del_rec(node->left);
    bst_del_rec(node->right);
    free(node);
}

static void bst_fprint_rec(FILE *file, const struct bstnode *node)
{
    if (!node)
    {
        fprintf(file, "n");
        return;
    }

    fprintf(file, "(");
    bst_fprint_rec(file, node->left);
    fprintf(file, ", %d, ", node->value);
    bst_fprint_rec(file, node->right);
    fprintf(file, ")");
}

void bst_init(struct bst *bst)
{
    bst->root = NULL;
}

void bst_del(struct bst *bst)
{
    bst_del_rec(bst->root);
    bst->root = NULL;
}

enum bstresult bst_add(struct bst *bst, int value)
{
    struct bstnode **current = &bst->root;
    while (*current)
    {
        if (value < (*current)->value)
            current = &(*current)->left;
        else if (value > (*current)->value)
            current = &(*current)->right;
        else
            return BSTRESULT_FAILURE_VALUE_EXISTS;
    }

    struct bstnode *node = malloc(sizeof(*node));
    if (!node)
        return BSTRESULT_FAILURE_ALLOCATION;

    bstnode_init(node, value);
    *current = node;

    return BSTRESULT_SUCCESS;
}

bool bst_in(const struct bst *bst, int value)
{
    const struct bstnode *current = bst->root;
    while (current)
    {
        if (value < current->value)
            current = current->left;
        else if (value > current->value)
            current = current->right;
        else
            return true;
    }

    return false;
}

bool bst_rmv(struct bst *bst, int value)
{
    struct bstnode **current = &bst->root;
    while (*current)
    {
        if (value < (*current)->value)
            current = &(*current)->left;
        else if (value > (*current)->value)
            current = &(*current)->right;
        else
            break;
    }

    // The value is not inside the BST.
    if (!*current)
        return false;

    if ((*current)->left && (*current)->right)
    {
        // Find in-order successor.
        struct bstnode **successor = &(*current)->right;
        while ((*successor)->left)
            successor = &(*successor)->left;

        // Copy value of in-order successor to node of removed value.
        (*current)->value = (*successor)->value;

        // Free the successor node of the copied value.
        struct bstnode *removed = *successor;
        *successor = removed->right;
        free(removed);
    }
    else if ((*current)->left || (*current)->right)
    {
        // Remove the node and fill the gap with the only child.
        struct bstnode *removed = *current;

        if (removed->left)
            *current = removed->left;
        else
            *current = removed->right;

        free(removed);
    }
    else
    {
        // Remove the leaf node.
        free(*current);
        *current = NULL;
    }

    return true;
}

void bst_fprint(FILE *file, const struct bst *bst)
{
    bst_fprint_rec(file, bst->root);
    fprintf(file, "\n");
}