/* AUTOGENERATED FILE! DO NOT EDIT! */
/*
 *  The Hilbert Kernel Library, a library for verifying formal proofs.
 *  Copyright © 2011 Alexander Klauer
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  To contact the author
 *     by email: Graf.Zahl@gmx.net
 *     on wiki : http://www.wikiproofs.org/w/index.php?title=User_talk:GrafZahl
 */

/**
 * Replacements from map template:
 * - <code>SubstitutionMap</code>:
 *   Map typename.
 * - <code>HilbertHandle</code>:
 *   Type of the mapping keys.
 * - <code>struct HilbertExpression *</code>:
 *   Type of the mapping values.
 * - <code>SubstitutionMapEntry</code>:
 *   Type of the map entries.
 * - <code>SubstitutionMapIterator</code>:
 *   Map iterator type.
 * - <code>hilbert_smap</code>:
 *   Function name prefix.
 * - <code>cl_hash32</code>:
 *   Name of hash function to be used.
 */

#ifndef HILBERT_CL_SubstitutionMap_H__
#define HILBERT_CL_SubstitutionMap_H__

#include<assert.h>
#include<stdint.h>
#include<stdlib.h>

#include"hash.h"

/**
 * Initial number of buckets.
 */
#define CL_SubstitutionMap_NUMBUCKETS 4

/**
 * Maximum load coefficient.
 */
#define CL_SubstitutionMap_MAXLOAD 4

/**
 * Bucket states.
 */
enum CLSubstitutionMapBucketState {
	CL_SubstitutionMap_BUCKET_EMPTY = 0,
	CL_SubstitutionMap_BUCKET_OCCUPIED,
	CL_SubstitutionMap_BUCKET_DELETED
};

/**
 * Map entry.
 */
struct SubstitutionMapEntry {
	/**
	 * Mapping key.
	 */
	HilbertHandle key;

	/**
	 * Mapping value.
	 */
	struct HilbertExpression * value;
};

/**
 * Bucket type.
 */
struct SubstitutionMapBucket {
	/**
	 * Map entry.
	 */
	struct SubstitutionMapEntry entry;

	/**
	 * Current bucket state.
	 */
	enum CLSubstitutionMapBucketState state;
};

/**
 * Map structure.
 */
struct SubstitutionMap {
	/**
	 * Number of entries.
	 */
	size_t count;

	/**
	 * Load threshold.
	 */
	size_t threshold;

	/**
	 * Total number of buckets minus one.
	 */
	size_t sizemask;

	/**
	 * Mapping buckets.
	 */
	struct SubstitutionMapBucket * buckets;
};

typedef struct SubstitutionMap SubstitutionMap;

/**
 * Map iterator structure.
 */
struct SubstitutionMapIterator {
	/**
	 * Pointer to map over which we are iterating.
	 */
	SubstitutionMap * map;

	/**
	 * Current index.
	 */
	size_t index;
};

typedef struct SubstitutionMapIterator SubstitutionMapIterator;

/**
 * Creates a new, empty map.
 *
 * @return On success, a pointer to a new, empty map is returned.
 * 	On error, <code>NULL</code> is returned.
 */
static inline SubstitutionMap * hilbert_smap_new(void) {
	SubstitutionMap * result;

	result = malloc(sizeof(*result));
	if (result == NULL)
		goto nomapmem;

	result->count = 0;
	result->threshold = (CL_SubstitutionMap_MAXLOAD - 1) * CL_SubstitutionMap_NUMBUCKETS / CL_SubstitutionMap_MAXLOAD;
	result->sizemask = CL_SubstitutionMap_NUMBUCKETS - 1;

	result->buckets = calloc(CL_SubstitutionMap_NUMBUCKETS, sizeof(*result->buckets));
	if (result->buckets == NULL)
		goto nobucketmem;

	return result;

nobucketmem:
	free(result);
nomapmem:
	return NULL;
}

/**
 * Deletes a map.
 *
 * @param map Pointer to a map.
 */
static inline void hilbert_smap_del(struct SubstitutionMap * map) {
	assert (map != NULL);

	free(map->buckets);
	free(map);
}

/**
 * Stores a mapping in the correct bucket (private).
 *
 * @param buckets Pointer to an array of buckets. At least one bucket in the array must be unoccupied.
 * @param sizemask Size of the array pointed to by <code>buckets</code>, minus one.
 * @param entry Mapping entry to be stored. No entry with the same key must be present in any of the occupied buckets.
 * @param hash Hash code of the key of <code>mapping</code>.
 */
static inline void hilbert_smap_store(struct SubstitutionMapBucket * buckets, size_t sizemask, struct SubstitutionMapEntry entry, size_t hash) {
	assert (buckets != NULL);
	assert (sizemask != 0);

	for (size_t i = hash & sizemask; 1; i = (i + 1) & sizemask) {
		if (buckets[i].state != CL_SubstitutionMap_BUCKET_OCCUPIED) {
			buckets[i].entry = entry;
			buckets[i].state = CL_SubstitutionMap_BUCKET_OCCUPIED;
			break;
		}
	}
}

/**
 * Find a candidate bucket (private)
 *
 * @param buckets Pointer to an array of buckets. At least one bucket in the array must be unoccupied.
 * @param sizemask Size of the array pointed to by <code>buckets</code>, minus one.
 * @param key Key to search candidate bucket for.
 * @param hash Hash code of <code>key/code>.
 *
 * @return A pointer to an element of the array pointed to by <code>buckets</code> is returned.
 * 	If the element is occupied, it is occupied with a mapping with key <code>key</code>.
 * 	Otherwise, the element is suitable for storing a mapping with key <code>key</code>.
 */
static inline struct SubstitutionMapBucket * hilbert_smap_find(struct SubstitutionMapBucket * buckets, size_t sizemask, HilbertHandle key, size_t hash) {
	assert (buckets != NULL);
	assert (sizemask > 0);
	assert (sizemask < SIZE_MAX);
	assert ((sizemask & (sizemask + 1)) == 0);

	int deletedfound = 0;
	struct SubstitutionMapBucket * firstdeleted;
	size_t count = 0;
	for (size_t i = hash & sizemask; count <= sizemask; i = (i + 1) & sizemask, ++count) {
		switch (buckets[i].state) {
			case CL_SubstitutionMap_BUCKET_EMPTY:
				if (deletedfound) {
					return firstdeleted;
				} else {
					return &buckets[i];
				}
				break;
			case CL_SubstitutionMap_BUCKET_OCCUPIED:
				if (buckets[i].entry.key == key)
					return &buckets[i];
				break;
			case CL_SubstitutionMap_BUCKET_DELETED:
				if (!deletedfound) {
					deletedfound = 1;
					firstdeleted = &buckets[i];
				}
				break;
		}
	}

	assert (deletedfound);
	return firstdeleted;
}

/**
 * Sets a value for a key in a map.
 * If a mapping for the specified key already exists, its old value will be overwritten with the specified value.
 *
 * @param map Pointer to a map.
 * @param key Key from the map domain for which a value is to be set.
 * @param value Value the new mapping should map <code>key</code> to.
 *
 * @return On success, <code>0</code> is returned.
 * 	On error, <code>-1</code> is returned.
 */
static inline int hilbert_smap_set(SubstitutionMap * map, HilbertHandle key, struct HilbertExpression * value) {
	assert (map != NULL);

	struct SubstitutionMapEntry entry = { .key = key, .value = value };
	size_t hash = cl_hash32(key);
	struct SubstitutionMapBucket * candidate = hilbert_smap_find(map->buckets, map->sizemask, key, hash);
	if (candidate->state == CL_SubstitutionMap_BUCKET_OCCUPIED) { /* mapping already present */
		candidate->entry.value = value;
		return 0;
	}

	assert (map->count < SIZE_MAX);
	size_t newcount = map->count + 1;
	if (newcount > map->threshold) {
		/* rebuild table */
		size_t oldalloc = (map->sizemask + 1) * sizeof(*map->buckets);
		size_t newalloc = 2 * oldalloc;
		if (newalloc <= oldalloc)
			return -1;
		size_t newsize = newalloc / sizeof(*map->buckets);
		size_t newsizemask = newsize - 1;
		size_t newthreshold = (CL_SubstitutionMap_MAXLOAD - 1) * newsize / CL_SubstitutionMap_MAXLOAD;
		struct SubstitutionMapBucket * newbuckets = calloc(1, newalloc);
		if (newbuckets == NULL)
			return -1;
		for (size_t i = 0; i <= map->sizemask; ++i) {
			if (map->buckets[i].state == CL_SubstitutionMap_BUCKET_OCCUPIED) {
				hilbert_smap_store(newbuckets, newsizemask, map->buckets[i].entry,
						cl_hash32(map->buckets[i].entry.key));
			}
		}
		free(map->buckets);
		map->sizemask = newsizemask;
		map->threshold = newthreshold;
		map->buckets = newbuckets;

		/* store mapping */
		hilbert_smap_store(map->buckets, map->sizemask, entry, hash);
		map->count = newcount;
		return 0;
	}

	/* store mapping */
	candidate->entry = entry;
	candidate->state = CL_SubstitutionMap_BUCKET_OCCUPIED;
	map->count = newcount;
	return 0;
}

/**
 * Obtains the value for a key.
 *
 * @param map Pointer to a map.
 * @param key Key whose associated value is to be obtained.
 *
 * @return If a mapping with key <code>key</code> exists, a pointer to the associated value is returned.
 * 	The value may be altered through this pointer. However, the returned pointer is guaranteed to be valid
 * 	only until the next call to one of the map functions with <code>map</code> as argument.
 * 	If no mapping with key <code>key</code> exists, <code>NULL</code> is returned.
 */
static inline struct HilbertExpression * * hilbert_smap_get(const SubstitutionMap * map, HilbertHandle key) {
	assert (map != NULL);

	struct SubstitutionMapBucket * candidate = hilbert_smap_find(map->buckets, map->sizemask, key, cl_hash32(key));
	if (candidate->state != CL_SubstitutionMap_BUCKET_OCCUPIED)
		return NULL;

	return &candidate->entry.value;
}

/**
 * Creates a new map iterator.
 *
 * @param map Pointer to a map.
 *
 * @return An iterator for the map pointed to by <code>map</code> is returned.
 */
static inline SubstitutionMapIterator hilbert_smap_iterator_new(SubstitutionMap * map) {
	assert (map != NULL);

	return (SubstitutionMapIterator) { .map = map, .index = 0 };
}

/**
 * Checks whether an iterator has a next element.
 *
 * @param i Pointer to map iterator.
 *
 * @return If there is a next element in the iteration, <code>1</code> is returned.
 * 	Otherwise, <code>0</code> is returned.
 */
static inline int hilbert_smap_iterator_hasnext(SubstitutionMapIterator * i) {
	assert (i != NULL);

	for(; i->index <= i->map->sizemask; ++i->index)
		if (i->map->buckets[i->index].state == CL_SubstitutionMap_BUCKET_OCCUPIED)
			return 1;
	return 0;
}

/**
 * Returns the next mapping in an iteration.
 *
 * @param i Pointer to map iterator.
 *
 * @return the next mapping in the iteration is returned.
 * 	If there is no next mapping, the behaviour is undefined.
 */
static inline struct SubstitutionMapEntry hilbert_smap_iterator_next(SubstitutionMapIterator * i) {
	assert (i != NULL);

	for (;; ++i->index) {
		assert (i->index <= i->map->sizemask);
		if (i->map->buckets[i->index].state == CL_SubstitutionMap_BUCKET_OCCUPIED)
			return i->map->buckets[i->index++].entry;
	}
}

#endif
