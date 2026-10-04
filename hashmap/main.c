#include <stdio.h>

#include "hashmap.h"

int main(void)
{
    struct hashmap hm = {0};
    hashmap_initialize(&hm);

    for (size_t i = 0; i < 1000000; i++)
        hashmap_put(&hm, i, "test");
    printf("bins: %zu\n", hm.bins_count);

    const char *value;
    if (hashmap_tryget(&hm, 666, &value))
        printf("key 666 contains value \"%s\"\n", value);

    hashmap_remove(&hm, 666);

    if (!hashmap_tryget(&hm, 666, NULL))
        printf("key 666 does not exist\n");

    hashmap_delete(&hm);

    return 0;
}