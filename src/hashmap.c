#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "hashmap.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * this should be prime
 */
#define TABLE_STARTSIZE 1021

#define ACTIVE 1

static unsigned long isPrime(identifier_t val)
{
    int i;
    identifier_t a, p, exp;

    for (i = 9; i--;)
    {
        a = (rand() % (val - 4)) + 2;
        p = 1;
        exp = val - 1;

        while (exp)
        {

            if (exp & 1) {
                p = (p * a) % val;
            }
            a = (a * a) % val;
            exp >>= 1;
        }

        if (p != 1) {
            return 0;
        }
    }
    return 1;
}

static unsigned long findPrimeGreaterThan(unsigned long val)
{

    if (val & 1) {
        val += 2;
    }

    else {
        val++;
    }

    while (!isPrime(val)) {
        val += 2;
    }
    return val;
}

static void rehash(hashmap *hm)
{
    long size = hm->size;
    hash_entry *table = hm->table;
    hm->size = findPrimeGreaterThan(size << 1);
    hm->table = (hash_entry *) calloc(sizeof(hash_entry), hm->size);
    hm->count = 0;

    while (--size >= 0)

        if (table[size].flags == ACTIVE) {
            hm_insert(hm, table[size].data, table[size].key);
        }
    free(table);
}

/*
 * Returns a hash code for the provided string.
 */
static unsigned long hash_key(const char *str)
{
    unsigned long hash = 5381;
    int c;

    while ((c = *str++))
    {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

hashmap *new_hashmap(long startsize)
{
    hashmap *hm = (hashmap *) malloc(sizeof(hashmap));

    if (!startsize) {
        startsize = TABLE_STARTSIZE;
    }

    else {
        startsize = findPrimeGreaterThan(startsize - 2);
    }
    hm->table = (hash_entry *) calloc(sizeof(hash_entry), startsize);
    hm->size = startsize;
    hm->count = 0;
    return hm;
}

void hm_foreach(hashmap *hash, void (*foreach) (void *data))
{

    if (hash->count)
    {

        for (int i = 0; i < hash->size; i++)
        {

            if (!hash->table[i].data) {
                break;
            }
            (foreach) (hash->table[i].data);
        }
    }
}

void *hm_start(hashmap *hm)
{
    hm->pos = 0;

    if (hm->pos < hm->size)
    {
        return hm->table[hm->pos].data;
    }
    return 0;
}

void *hm_next(hashmap *hm)
{

    if (++hm->pos < hm->size) {
        return hm->table[hm->pos].data;
    }
    return 0;
}

bool hm_hasnext(hashmap *hm)
{
    return hm->pos < hm->size && hm->table[hm->pos].data;
}

void sm_insert(hashmap *hash, const void *data, const char *key)
{
    hm_insert(hash, data, hash_key(key));
}

void hm_insert(hashmap *hash, const void *data, identifier_t key)
{
    identifier_t index, i, step;

    if (hash->size <= hash->count) {
        rehash(hash);
    }

    do
    {
        index = key % hash->size;
        step = (key % (hash->size - 2)) + 1;

        for (i = 0; i < hash->size; i++)
        {

            if (hash->table[index].flags & ACTIVE)
            {

                if (hash->table[index].key == key)
                {
                    hash->table[index].data = (void *)data;
                    return;
                }
            }

            else
            {
                hash->table[index].flags |= ACTIVE;
                hash->table[index].data = (void *)data;
                hash->table[index].key = key;
                ++hash->count;
                return;
            }
            index = (index + step) % hash->size;
        }
        /*
         * it should not be possible that we EVER come this far, but
         * unfortunately not every generated prime number is prime
         * (Carmichael numbers...)
         */
        rehash(hash);
    }

    while (1);
}

void *sm_remove(hashmap *hash, const char *key)
{
    return hm_remove(hash, hash_key(key));
}

void *hm_remove(hashmap *hash, identifier_t key)
{
    identifier_t index, i, step;
    index = key % hash->size;
    step = (key % (hash->size - 2)) + 1;

    for (i = 0; i < hash->size; i++)
    {

        if (hash->table[index].data)
        {

            if (hash->table[index].key == key)
            {

                if (hash->table[index].flags & ACTIVE)
                {
                    hash->table[index].flags &= ~ACTIVE;
                    --hash->count;
                    return hash->table[index].data;
                }

                else {  /* in, but not active (i.e. deleted) */
                    return 0;
                }
            }
        }

        else {      /* found an empty place (can't be in) */
            return 0;
        }
        index = (index + step) % hash->size;
    }
    /*
     * everything searched through, but not in
     */
    return 0;
}

void *sm_get(hashmap *hash, const char *key)
{
    return hm_get(hash, hash_key(key));
}

void *hm_get(hashmap *hash, identifier_t key)
{

    if (hash->count)
    {
        identifier_t index, i, step;
        index = key % hash->size;
        step = (key % (hash->size - 2)) + 1;

        for (i = 0; i < hash->size; i++)
        {

            if (hash->table[index].key == key)
            {

                if (hash->table[index].flags & ACTIVE) {
                    return hash->table[index].data;
                }
                break;
            }

            else if (!hash->table[index].data) {
                break;
            }
            index = (index + step) % hash->size;
        }
    }
    return 0;
}

long hm_count(hashmap *hash)
{
    return hash->count;
}

void destroy_hashmap(hashmap *hash)
{
    free(hash->table);
    free(hash);
}
