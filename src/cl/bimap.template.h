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
 * Replacements from bimap template:
 * - <code>BIMAP</code>:
 *   Bimap typename.
 * - <code>DOM_TYPE</code>:
 *   Type of the bimap's domain.
 * - <code>COD_TYPE</code>:
 *   Type of the bimap's codomain.
 * - <code>ENTRY_TYPE</code>:
 *   Type of the bimap entries.
 * - <code>BMITER</code>:
 *   Bimap iterator type.
 * - <code>PREFIX</code>:
 *   Function name prefix.
 * - <code>DOM_HASH</code>:
 *   Name of domain hash function.
 * - <code>COD_HASH</code>:
 *   Name of codomain hash function.
 */

#ifndef HILBERT_CL_BIMAP_H__
#define HILBERT_CL_BIMAP_H__

#include<assert.h>
#include<stdint.h>
#include<stdlib.h>

#include"hash.h"

/**
 * Initial number of buckets.
 */
#define CL_BIMAP_NUMBUCKETS 4

/**
 * Maximum load coefficient.
 */
#define CL_BIMAP_MAXLOAD 4

/**
 * Bucket states.
 */
enum CLBIMAPBucketState {
	CL_BIMAP_BUCKET_EMPTY = 0,
	CL_BIMAP_BUCKET_OCCUPIED,
	CL_BIMAP_BUCKET_DELETED
};

/**
 * Bimap entry.
 */
struct ENTRY_TYPE {
	/**
	 * Preimage.
	 */
	DOM_TYPE pre;

	/**
	 * Image.
	 */
	COD_TYPE post;
};

/**
 * Bucket type.
 */
struct BIMAPBucket {
	/**
	 * Bimap entry.
	 */
	struct ENTRY_TYPE entry;

	/**
	 * Current bucket state.
	 */
	enum CLBIMAPBucketState state;
};

/**
 * Map structure.
 */
struct BIMAP {
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
	 * Domain based mapping buckets.
	 */
	struct BIMAPBucket * dom_buckets;

	/**
	 * Codomain based mapping buckets.
	 */
	struct BIMAPBucket * cod_buckets;
};

typedef struct BIMAP BIMAP;

/**
 * Bimap iterator structure.
 */
struct BMITER {
	/**
	 * Pointer to bimap over which we are iterating.
	 */
	BIMAP * bimap;

	/**
	 * Current index (domain based).
	 */
	size_t index;
};

typedef struct BMITER BMITER;

/**
 * Creates a new, empty bimap.
 *
 * @return On success, a pointer to a new, empty bimap is returned.
 * 	On error, <code>NULL</code> is returned.
 */
static inline BIMAP * PREFIX_new(void) {
	BIMAP * result;

	result = malloc(sizeof(*result));
	if (result == NULL)
		goto nomapmem;

	result->count = 0;
	result->threshold = (CL_BIMAP_MAXLOAD - 1) * CL_BIMAP_NUMBUCKETS / CL_BIMAP_MAXLOAD;
	result->sizemask = CL_BIMAP_NUMBUCKETS - 1;

	result->dom_buckets = calloc(CL_BIMAP_NUMBUCKETS, sizeof(*result->dom_buckets));
	if (result->dom_buckets == NULL)
		goto nodommem;

	result->cod_buckets = calloc(CL_BIMAP_NUMBUCKETS, sizeof(*result->dom_buckets));
	if (result->cod_buckets == NULL)
		goto nocodmem;

	return result;

nocodmem:
	free(result->dom_buckets);
nodommem:
	free(result);
nomapmem:
	return NULL;
}

/**
 * Frees a bimap.
 *
 * @param bimap Pointer to a bimap.
 */
static inline void PREFIX_del(BIMAP * bimap) {
	assert (bimap != NULL);

	free(bimap->cod_buckets);
	free(bimap->dom_buckets);
	free(bimap);
}

/**
 * Stores a mapping in the correct bucket (private).
 *
 * @param buckets Pointer to an array of buckets. At least one bucket in the array must be unoccupied.
 * @param sizemask Size of the array pointed to by <code>buckets</code>, minus one.
 * @param entry Mapping entry to be stored. No entry with the same key (preimage or postimage, depending on
 * 	bucket array) must be present in any of the occupied buckets.
 * @param hash Hash code of the key (preimage or postimage) of <code>mapping</code>.
 */
static inline void PREFIX_store(struct BIMAPBucket * buckets, size_t sizemask, struct ENTRY_TYPE entry, size_t hash) {
	assert (buckets != NULL);
	assert (sizemask > 0);
	assert (sizemask < SIZE_MAX);
	assert (((sizemask + 1) & sizemask) == 0);

	for (size_t i = hash & sizemask; 1; i = (i + 1) & sizemask) {
		if (buckets[i].state != CL_BIMAP_BUCKET_OCCUPIED) {
			buckets[i].entry = entry;
			buckets[i].state = CL_BIMAP_BUCKET_OCCUPIED;
			break;
		}
	}
}

/**
 * Finds a candidate bucket for preimage-based mapping (private).
 *
 * @param buckets Pointer to an array of buckets. At least one bucket in the array must be unoccupied.
 * @param sizemask Size of the array pointed to by <code>buckets</code>, minus one.
 * @param pre Preimage to search candidate bucket for.
 * @param hash Hash code of <code>pre</code>.
 *
 * @return A pointer to an element of the array pointed to by <code>buckets</code> is returned.
 * 	If the element is occupied, it is occupied with a mapping with preimage <code>pre</code>.
 * 	Otherwise, the element is suitable for storing a mapping with preimage <code>pre</code>.
 */
static inline struct BIMAPBucket * PREFIX_find_pre(struct BIMAPBucket * buckets, size_t sizemask, DOM_TYPE pre,
		size_t hash) {
	assert (buckets != NULL);
	assert (sizemask > 0);
	assert (sizemask < SIZE_MAX);
	assert ((sizemask & (sizemask + 1)) == 0);

	int deletedfound = 0;
	struct BIMAPBucket * firstdeleted;
	size_t count = 0;
	for (size_t i = hash & sizemask; count <= sizemask; i = (i + 1) & sizemask, ++count) {
		switch (buckets[i].state) {
			case CL_BIMAP_BUCKET_EMPTY:
				if (deletedfound) {
					return firstdeleted;
				} else {
					return &buckets[i];
				}
				break;
			case CL_BIMAP_BUCKET_OCCUPIED:
				if (buckets[i].entry.pre == pre)
					return &buckets[i];
				break;
			case CL_BIMAP_BUCKET_DELETED:
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
 * Finds a candidate bucket for postimage-based mapping (private).
 *
 * @param buckets Pointer to an array of buckets. At least one bucket in the array must be unoccupied.
 * @param sizemask Size of the array pointed to by <code>buckets</code>, minus one.
 * @param post Postimage to search candidate bucket for.
 * @param hash Hash code of <code>post</code>.
 *
 * @return A pointer to an element of the array pointed to by <code>buckets</code> is returned.
 * 	If the element is occupied, it is occupied with a mapping with postimage <code>post</code>.
 * 	Otherwise, the element is suitable for storing a mapping with postimage <code>post</code>.
 */
static inline struct BIMAPBucket * PREFIX_find_post(struct BIMAPBucket * buckets, size_t sizemask, COD_TYPE post,
		size_t hash) {
	assert (buckets != NULL);
	assert (sizemask > 0);
	assert (sizemask < SIZE_MAX);
	assert ((sizemask & (sizemask + 1)) == 0);

	int deletedfound = 0;
	struct BIMAPBucket * firstdeleted;
	size_t count = 0;
	for (size_t i = hash & sizemask; count <= sizemask; i = (i + 1) & sizemask, ++count) {
		switch (buckets[i].state) {
			case CL_BIMAP_BUCKET_EMPTY:
				if (deletedfound) {
					return firstdeleted;
				} else {
					return &buckets[i];
				}
				break;
			case CL_BIMAP_BUCKET_OCCUPIED:
				if (buckets[i].entry.post == post)
					return &buckets[i];
				break;
			case CL_BIMAP_BUCKET_DELETED:
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
 * Adds an entry to the bimap.
 * If an entry with the same preimage but a different postimage, or vice-versa, already exists,
 * it will be overwritten, ensuring that the entries continue to constitute a bijective map.
 * Warning: as a result of this policy, the bimap may end up with less entries than before the addition operation.
 *
 * @param bimap Pointer to a bimap.
 * @param pre Preimage.
 * @param post Postimage.
 *
 * @return On success, <code>0</code> is returned.
 * 	On error, <code>-1</code> is returned and the bimap remains unchanged.
 */
static inline int PREFIX_add(BIMAP * bimap, DOM_TYPE pre, COD_TYPE post) {
	assert (bimap != NULL);

	struct ENTRY_TYPE entry = { .pre = pre, .post = post };
	size_t pre_hash = DOM_HASH(pre);
	size_t post_hash = COD_HASH(post);

	/* delete old entries if present */
	struct BIMAPBucket * pre_cand = PREFIX_find_pre(bimap->dom_buckets, bimap->sizemask, pre, pre_hash);
	if (pre_cand->state == CL_BIMAP_BUCKET_OCCUPIED) {
		struct BIMAPBucket * post = PREFIX_find_post(bimap->cod_buckets, bimap->sizemask,
				pre_cand->entry.post, COD_HASH(pre_cand->entry.post));
		assert (post->state == CL_BIMAP_BUCKET_OCCUPIED);
		pre_cand->state = CL_BIMAP_BUCKET_DELETED;
		post->state = CL_BIMAP_BUCKET_DELETED;
		--bimap->count;
	}
	struct BIMAPBucket * post_cand = PREFIX_find_post(bimap->cod_buckets, bimap->sizemask, post, post_hash);
	if (post_cand->state == CL_BIMAP_BUCKET_OCCUPIED) {
		struct BIMAPBucket * pre = PREFIX_find_pre(bimap->dom_buckets, bimap->sizemask,
				post_cand->entry.pre, DOM_HASH(post_cand->entry.pre));
		assert (pre->state == CL_BIMAP_BUCKET_OCCUPIED);
		post_cand->state = CL_BIMAP_BUCKET_DELETED;
		pre->state = CL_BIMAP_BUCKET_DELETED;
		--bimap->count;
	}

	/* add entry */
	assert (bimap->count < SIZE_MAX);
	size_t newcount = bimap->count + 1;
	if (newcount > bimap->threshold) { /* this can only happen if we didn't delete anything above */
		/* rebuild tables */
		size_t oldalloc = (bimap->sizemask + 1) * sizeof(*bimap->dom_buckets);
		size_t newalloc = 2 * oldalloc;
		if (newalloc <= oldalloc)
			return -1;
		size_t newsize = newalloc / sizeof(*bimap->dom_buckets);
		size_t newsizemask = newsize - 1;
		size_t newthreshold = (CL_BIMAP_MAXLOAD - 1) * newsize / CL_BIMAP_MAXLOAD;
		struct BIMAPBucket * newdombuckets = calloc(1, newalloc);
		struct BIMAPBucket * newcodbuckets = calloc(1, newalloc);
		if ((newdombuckets == NULL) || (newcodbuckets == NULL)) {
			free(newdombuckets);
			free(newcodbuckets);
			return -1;
		}
		for (size_t i = 0; i <= bimap->sizemask; ++i) {
			if (bimap->dom_buckets[i].state == CL_BIMAP_BUCKET_OCCUPIED) {
				PREFIX_store(newdombuckets, newsizemask, bimap->dom_buckets[i].entry,
						DOM_HASH(bimap->dom_buckets[i].entry.pre));
			}
			if (bimap->cod_buckets[i].state == CL_BIMAP_BUCKET_OCCUPIED) {
				PREFIX_store(newcodbuckets, newsizemask, bimap->cod_buckets[i].entry,
						COD_HASH(bimap->cod_buckets[i].entry.post));
			}
		}
		free(bimap->cod_buckets);
		free(bimap->dom_buckets);
		bimap->sizemask = newsizemask;
		bimap->threshold = newthreshold;
		bimap->dom_buckets = newdombuckets;
		bimap->cod_buckets = newcodbuckets;

		/* store mapping */
		PREFIX_store(bimap->dom_buckets, bimap->sizemask, entry, pre_hash);
		PREFIX_store(bimap->cod_buckets, bimap->sizemask, entry, post_hash);
		bimap->count = newcount;
		return 0;
	}

	/* store mapping */
	pre_cand->entry = entry;
	pre_cand->state = CL_BIMAP_BUCKET_OCCUPIED;
	post_cand->entry = entry;
	post_cand->state = CL_BIMAP_BUCKET_OCCUPIED;
	bimap->count = newcount;
	return 0;
}

/**
 * Obtains the image for a given preimage.
 *
 * @param bimap Pointer to a bimap.
 * @param pre Preimage.
 *
 * @return If a mapping for the given preimage exists, a pointer to the associated image is returned.
 * 	The value being pointed to may not be altered (use <code>#PREFIX_add()</code> for this purpose).
 * 	The returned pointer is guaranteed to be valid only until the next call to one of the bimap
 * 	functions with <code>bimap</code> as argument.
 * 	If no mapping for the given preimage exists, <code>NULL</code> is returned.
 */
static inline const COD_TYPE * PREFIX_post(const BIMAP * bimap, DOM_TYPE pre) {
	assert (bimap != NULL);

	struct BIMAPBucket * candidate = PREFIX_find_pre(bimap->dom_buckets, bimap->sizemask, pre, DOM_HASH(pre));
	if (candidate->state == CL_BIMAP_BUCKET_OCCUPIED)
		return &candidate->entry.post;

	return NULL;
}

/**
 * Obtains the preimage for a given image.
 *
 * @param bimap Pointer to a bimap.
 * @param post Image.
 *
 * @return If a mapping for the given image exists, a pointer to the associated preimage is returned.
 * The value being pointed to may not be altered (use <code>#PREFIX_add()</code> for this purpose).
 * The returned pointer is guaranteed to be valid only until the next call to one of the bimap
 * functions with <code>bimap</code> as argument.
 * If no mapping for the given image exists, <code>NULL</code> is returned.
 */
static inline const DOM_TYPE * PREFIX_pre(const BIMAP * bimap, COD_TYPE post) {
	assert (bimap != NULL);

	struct BIMAPBucket * candidate = PREFIX_find_post(bimap->cod_buckets, bimap->sizemask, post, COD_HASH(post));
	if (candidate->state == CL_BIMAP_BUCKET_OCCUPIED)
		return &candidate->entry.pre;

	return NULL;
}

/**
 * Creates a new bimap iterator.
 *
 * @param bimap Pointer to a bimap.
 *
 * @return An iterator for the map pointed to by <code>bimap</code> is returned.
 */
static inline BMITER PREFIX_iterator_new(BIMAP * bimap) {
	assert (bimap != NULL);

	return (BMITER) { .bimap = bimap, .index = 0 };
}

/**
 * Checks whether an iterator has a next element.
 *
 * @return i Pointer to bimap iterator.
 *
 * @return If there is a next element in the iteration, <code>1</code> is returned.
 * 	Otherwise, <code>0</code> is returned.
 */
static inline int PREFIX_iterator_hasnext(BMITER * i) {
	assert (i != NULL);

	for (; i->index <= i->bimap->sizemask; ++i->index) {
		if (i->bimap->dom_buckets[i->index].state == CL_BIMAP_BUCKET_OCCUPIED)
			return 1;
	}

	return 0;
}

/**
 * Returns the next mapping in an iteration.
 *
 * @param i Pointer to bimap iterator.
 *
 * @return the next mapping in the iteration is returned.
 * 	If there is no next mapping, the behaviour is undefined.
 */
static inline struct ENTRY_TYPE PREFIX_iterator_next(BMITER * i) {
	assert (i != NULL);

	for (;; ++i->index) {
		assert (i->index <= i->bimap->sizemask);
		if (i->bimap->dom_buckets[i->index].state == CL_BIMAP_BUCKET_OCCUPIED)
			return i->bimap->dom_buckets[i->index++].entry;
	}
}

#endif
