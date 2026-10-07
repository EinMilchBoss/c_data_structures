#include <stdio.h>

#include "bst.h"

int main(void)
{
    struct bst bst;
    bst_init(&bst);

    bst_add(&bst, 0);
    bst_add(&bst, 20);
    bst_add(&bst, 10);
    bst_add(&bst, 30);
    bst_add(&bst, 25);
    bst_add(&bst, 22);

    bst_fprint(stdout, &bst);

    printf("%d = 1\n", bst_in(&bst, 1));
    printf("%d = 0\n", bst_in(&bst, -1));

    bst_rmv(&bst, 30);
    bst_fprint(stdout, &bst);

    bst_del(&bst);

    return 0;
}