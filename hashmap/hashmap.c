#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "hashmap.h"

/**
 * Every hash map starts with this amount of bins.
 *
 * NOTE: This has to be a power of 2 because we use a bit mask
 * instead of modulo for faster bin index calculations.
 */
static const size_t HASHMAP_INITIAL_BINS_COUNT = 16;

/**
 * When this threshold is reached, the amount of bins is doubled.
 */
static const double HASHMAP_INCREASE_BINS_LOAD_THRESHOLD = 0.75;

static void hashmapbinnode_delete(struct hashmapbinnode *hmb);

static void hashmapbinnode_delete(struct hashmapbinnode *hmbn)
{
    if (hmbn == NULL)
        return;

    free(hmbn->value);
    free(hmbn);
}

static bool hashmap_isinitialized(const struct hashmap *hm);
/**
 * NOTE: The bins count must be a power of 2!
 */
static size_t hashmap_getbinidx_withbinscount(size_t key, size_t bins_count);
static size_t hashmap_binidx(const struct hashmap *hm, size_t key);
static double hashmap_getloadfactor(const struct hashmap *hm);
static enum hashmapresult hashmap_enlarge(struct hashmap *hm_old);

static bool hashmap_isinitialized(const struct hashmap *hm)
{
    return hm->bins != NULL;
}

static size_t hashmap_getbinidx_withbinscount(size_t key, size_t bins_count)
{
    return key & (bins_count - 1);
}

static size_t hashmap_binidx(const struct hashmap *hm, size_t key)
{
    return hashmap_getbinidx_withbinscount(key, hm->bins_count);
}

static double hashmap_getloadfactor(const struct hashmap *hm)
{
    double load_factor = (double)hm->entries_count / hm->bins_count;
    return load_factor;
}

static enum hashmapresult hashmap_enlarge(struct hashmap *hm)
{
    if (hm->bins_count > SIZE_MAX / 2)
        return HASHMAPRESULT_FAILURE_OVERFLOW;

    size_t enlarged_bins_count = hm->bins_count * 2;
    struct hashmapbinnode **enlarged_bins = calloc(enlarged_bins_count, sizeof(*enlarged_bins));
    if (enlarged_bins == NULL)
        return HASHMAPRESULT_FAILURE_ALLOCATION;

    for (size_t bin_idx_old = 0; bin_idx_old < hm->bins_count; bin_idx_old++)
    {
        struct hashmapbinnode *current = hm->bins[bin_idx_old];
        while (current != NULL)
        {
            size_t bin_idx_new = hashmap_getbinidx_withbinscount(current->key, enlarged_bins_count);

            struct hashmapbinnode *saved_next = current->next;

            current->next = enlarged_bins[bin_idx_new];
            enlarged_bins[bin_idx_new] = current;

            current = saved_next;
        }
    }

    free(hm->bins);
    hm->bins = enlarged_bins;
    hm->bins_count = enlarged_bins_count;

    return HASHMAPRESULT_SUCCESS;
}

enum hashmapresult hashmap_initialize(struct hashmap *hm)
{
    if (hashmap_isinitialized(hm))
        return HASHMAPRESULT_FAILURE_BAD_INITIALIZATION;

    hm->bins = calloc(HASHMAP_INITIAL_BINS_COUNT, sizeof(*hm->bins));
    if (hm->bins == NULL)
        return HASHMAPRESULT_FAILURE_ALLOCATION;

    hm->bins_count = HASHMAP_INITIAL_BINS_COUNT;
    hm->entries_count = 0;

    return HASHMAPRESULT_SUCCESS;
}

void hashmap_delete(struct hashmap *hm)
{
    if (hashmap_isinitialized(hm))
    {
        for (size_t i = 0; i < hm->bins_count; i++)
        {
            struct hashmapbinnode *current = hm->bins[i];
            while (current != NULL)
            {
                struct hashmapbinnode *saved_next = current->next;
                hashmapbinnode_delete(current);

                current = saved_next;
            }
        }

        free(hm->bins);
        hm->bins = NULL;
    }

    hm->bins_count = 0;
    hm->entries_count = 0;
}

bool hashmap_tryget(
    const struct hashmap *hm,
    size_t key,
    const char **out_value)
{
    if (!hashmap_isinitialized(hm))
        return false;

    size_t bin_idx = hashmap_binidx(hm, key);

    struct hashmapbinnode *current = hm->bins[bin_idx];
    while (current != NULL)
    {
        if (current->key == key)
        {
            if (out_value != NULL)
                *out_value = current->value;

            return true;
        }

        current = current->next;
    }

    return false;
}

enum hashmapresult hashmap_put(struct hashmap *hm, size_t key, const char *value)
{
    if (!hashmap_isinitialized(hm))
        return HASHMAPRESULT_FAILURE_BAD_INITIALIZATION;

    if (hashmap_tryget(hm, key, NULL))
        return HASHMAPRESULT_FAILURE_KEY_EXISTS;

    if (hashmap_getloadfactor(hm) >= HASHMAP_INCREASE_BINS_LOAD_THRESHOLD)
    {
        enum hashmapresult result = hashmap_enlarge(hm);
        if (result)
            return result;
    }

    struct hashmapbinnode *new_node = malloc(sizeof(*new_node));
    if (new_node == NULL)
        return HASHMAPRESULT_FAILURE_ALLOCATION;

    char *value_cpy = strdup(value);
    if (value_cpy == NULL)
    {
        free(new_node);
        return HASHMAPRESULT_FAILURE_ALLOCATION;
    }

    size_t idx = hashmap_binidx(hm, key);
    *new_node = (struct hashmapbinnode){
        .key = key,
        .value = value_cpy,
        .next = hm->bins[idx],
    };
    hm->bins[idx] = new_node;
    hm->entries_count++;

    return HASHMAPRESULT_SUCCESS;
}

enum hashmapresult hashmap_remove(struct hashmap *hm, size_t key)
{
    if (!hashmap_isinitialized(hm))
        return HASHMAPRESULT_FAILURE_BAD_INITIALIZATION;

    size_t bin_idx = hashmap_binidx(hm, key);

    struct hashmapbinnode **current = &hm->bins[bin_idx];
    while (*current != NULL)
    {
        if ((*current)->key == key)
        {
            struct hashmapbinnode *deleted = *current;
            *current = (*current)->next;
            hashmapbinnode_delete(deleted);
            hm->entries_count--;

            return HASHMAPRESULT_SUCCESS;
        }

        current = &(*current)->next;
    }

    return HASHMAPRESULT_FAILURE_KEY_NOTEXISTS;
}

void hashmap_fprint(FILE *file, const struct hashmap *hm)
{
    if (!hashmap_isinitialized(hm))
        return;

    for (size_t i = 0; i < hm->bins_count; i++)
    {
        if (hm->bins[i] == NULL)
            continue;

        fprintf(file, "%zu: (%zu, %s)", i, hm->bins[i]->key, hm->bins[i]->value);

        struct hashmapbinnode *current = hm->bins[i]->next;
        while (current != NULL)
        {
            fprintf(file, " -> (%zu, %s)", current->key, current->value);
            current = current->next;
        }

        fprintf(file, "\n");
    }
}