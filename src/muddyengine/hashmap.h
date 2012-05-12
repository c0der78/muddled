
#ifndef HASHMAP_H_INCLUDED
#define HASHMAP_H_INCLUDED

#include <stdbool.h>
#include <muddyengine/engine.h>
#include <muddyengine/variant.h>

typedef struct {
	void *data;
	int flags;
	identifier_t key;
} hEntry;

/** Hashmap structure (forward declaration) */
struct s_hashmap {
	hEntry *table;
	long size, count, pos;
};

typedef struct s_hashmap hashmap;

/** Creates a new hashmap near the given size. */
extern hashmap *new_hashmap(long startsize);

/** Inserts a new element into the hashmap. */
extern void hm_insert(hashmap *, const void *data, identifier_t key);
extern void sm_insert(hashmap *, const void *data, const char *key);

/** Removes the storage for the element of the key and returns the element. */
extern void *hm_remove(hashmap *, identifier_t key);
extern void *sm_remove(hashmap *, const char *key);

/** Returns the element for the key. */
extern void *hm_get(hashmap *, identifier_t key);
extern void *sm_get(hashmap *, const char *key);

extern void hm_foreach(hashmap *, void (*foreach) (void *));

extern void *hm_start(hashmap *);
extern void *hm_next(hashmap *);
extern bool hm_hasnext(hashmap *);

/** Returns the number of saved elements. */
extern long hm_count(hashmap *);

/** Removes the hashmap structure. */
extern void destroy_hashmap(hashmap *);

#endif
