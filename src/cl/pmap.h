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
 * Replacements from bimap template:
 * - <code>ParamMap</code>:
 *   Bimap typename.
 * - <code>HilbertHandle</code>:
 *   Type of the bimap's domain.
 * - <code>HilbertHandle</code>:
 *   Type of the bimap's codomain.
 * - <code>ParamMapEntry</code>:
 *   Type of the bimap entries.
 * - <code>ParamMapIterator</code>:
 *   Bimap iterator type.
 * - <code>hilbert_pmap</code>:
 *   Function name prefix.
 * - <code>cl_hash32</code>:
 *   Name of domain hash function.
 * - <code>cl_hash32</code>:
 *   Name of codomain hash function.
 */

#ifndef HILBERT_CL_ParamMap_H__
#define HILBERT_CL_ParamMap_H__

#include<assert.h>
#include<stdint.h>
#include<stdlib.h>

#include"hash.h"

/**
 * Initial number of buckets.
 */
#define CL_ParamMap_NUMBUCKETS 4

/**
 * Maximum load coefficient.
 */
#define CL_ParamMap_MAXLOAD 4

/**
 * Bucket states.
 */
enum CLParamMapBucketState {
	CL_ParamMap_BUCKET_EMPTY = 0,
	CL_ParamMap_BUCKET_OCCUPIED,
	CL_ParamMap_BUCKET_DELETED
};

/**
 * Bimap entry.
 */
struct ParamMapEntry {
	/**
	 * Preimage.
	 */
	HilbertHandle pre;

	/**
	 * Image.
	 */
	HilbertHandle post;
};

/**
 * Bucket type.
 */
struct ParamMapBucket {
	/**
	 * Bimap entry.
	 */
	struct ParamMapEntry entry;

	/**
	 * Current bucket state.
	 */
	enum CLParamMapBucketState state;
};

/**
 * Map structure.
 */
struct ParamMap {
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
	struct ParamMapBucket * dom_buckets;

	/**
	 * Codomain based mapping buckets.
	 */
	struct ParamMapBucket * cod_buckets;
};

typedef struct ParamMap ParamMap;

/**
 * Bimap iterator structure.
 */
struct ParamMapIterator {
	/**
	 * Pointer to bimap over which we are iterating.
	 */
	ParamMap * bimap;

	/**
	 * Current index (domain based).
	 */
	size_t index;
};

typedef struct ParamMapIterator ParamMapIterator;

/**
 * Creates a new, empty bimap.
 *
 * @return On success, a pointer to a new, empty bimap is returned.
 * 	On error, <code>NULL</code> is returned.
 */
static inline ParamMap * hilbert_pmap_new(void) {
	ParamMap * result;

	result = malloc(sizeof(*result));
	if (result == NULL)
		goto nomapmem;

	result->count = 0;
	result->threshold = (CL_ParamMap_MAXLOAD - 1) * CL_ParamMap_NUMBUCKETS / CL_ParamMap_MAXLOAD;
	result->sizemask = CL_ParamMap_NUMBUCKETS - 1;

	result->dom_buckets = calloc(CL_ParamMap_NUMBUCKETS, sizeof(*result->dom_buckets));
	if (result->dom_buckets == NULL)
		goto nodommem;

	result->cod_buckets = calloc(CL_ParamMap_NUMBUCKETS, sizeof(*result->dom_buckets));
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
static inline void hilbert_pmap_del(ParamMap * bimap) {
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
static inline void hilbert_pmap_store(struct ParamMapBucket * buckets, size_t sizemask, struct ParamMapEntry entry, size_t hash) {
	assert (buckets != NULL);
	assert (sizemask > 0);
	assert (sizemask < SIZE_MAX);
	assert (((sizemask + 1) & sizemask) == 0);

	for (size_t i = hash & sizemask; 1; i = (i + 1) & sizemask) {
		if (buckets[i].state != CL_ParamMap_BUCKET_OCCUPIED) {
			buckets[i].entry = entry;
			buckets[i].state = CL_ParamMap_BUCKET_OCCUPIED;
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
static inline struct ParamMapBucket * hilbert_pmap_find_pre(struct ParamMapBucket * buckets, size_t sizemask, HilbertHandle pre,
		size_t hash) {
	assert (buckets != NULL);
	assert (sizemask > 0);
	assert (sizemask < SIZE_MAX);
	assert ((sizemask & (sizemask + 1)) == 0);

	int deletedfound = 0;
	struct ParamMapBucket * firstdeleted;
	size_t count = 0;
	for (size_t i = hash & sizemask; count <= sizemask; i = (i + 1) & sizemask, ++count) {
		switch (buckets[i].state) {
			case CL_ParamMap_BUCKET_EMPTY:
				if (deletedfound) {
					return firstdeleted;
				} else {
					return &buckets[i];
				}
				break;
			case CL_ParamMap_BUCKET_OCCUPIED:
				if (buckets[i].entry.pre == pre)
					return &buckets[i];
				break;
			case CL_ParamMap_BUCKET_DELETED:
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
static inline struct ParamMapBucket * hilbert_pmap_find_post(struct ParamMapBucket * buckets, size_t sizemask, HilbertHandle post,
		size_t hash) {
	assert (buckets != NULL);
	assert (sizemask > 0);
	assert (sizemask < SIZE_MAX);
	assert ((sizemask & (sizemask + 1)) == 0);

	int deletedfound = 0;
	struct ParamMapBucket * firstdeleted;
	size_t count = 0;
	for (size_t i = hash & sizemask; count <= sizemask; i = (i + 1) & sizemask, ++count) {
		switch (buckets[i].state) {
			case CL_ParamMap_BUCKET_EMPTY:
				if (deletedfound) {
					return firstdeleted;
				} else {
					return &buckets[i];
				}
				break;
			case CL_ParamMap_BUCKET_OCCUPIED:
				if (buckets[i].entry.post == post)
					return &buckets[i];
				break;
			case CL_ParamMap_BUCKET_DELETED:
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
static inline int hilbert_pmap_add(ParamMap * bimap, HilbertHandle pre, HilbertHandle post) {
	assert (bimap != NULL);

	struct ParamMapEntry entry = { .pre = pre, .post = post };
	size_t pre_hash = cl_hash32(pre);
	size_t post_hash = cl_hash32(post);

	/* delete old entries if present */
	struct ParamMapBucket * pre_cand = hilbert_pmap_find_pre(bimap->dom_buckets, bimap->sizemask, pre, pre_hash);
	if (pre_cand->state == CL_ParamMap_BUCKET_OCCUPIED) {
		struct ParamMapBucket * post = hilbert_pmap_find_post(bimap->cod_buckets, bimap->sizemask,
				pre_cand->entry.post, cl_hash32(pre_cand->entry.post));
		assert (post->state == CL_ParamMap_BUCKET_OCCUPIED);
		pre_cand->state = CL_ParamMap_BUCKET_DELETED;
		post->state = CL_ParamMap_BUCKET_DELETED;
		--bimap->count;
	}
	struct ParamMapBucket * post_cand = hilbert_pmap_find_post(bimap->cod_buckets, bimap->sizemask, post, post_hash);
	if (post_cand->state == CL_ParamMap_BUCKET_OCCUPIED) {
		struct ParamMapBucket * pre = hilbert_pmap_find_pre(bimap->dom_buckets, bimap->sizemask,
				post_cand->entry.pre, cl_hash32(post_cand->entry.pre));
		assert (pre->state == CL_ParamMap_BUCKET_OCCUPIED);
		post_cand->state = CL_ParamMap_BUCKET_DELETED;
		pre->state = CL_ParamMap_BUCKET_DELETED;
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
		size_t newthreshold = (CL_ParamMap_MAXLOAD - 1) * newsize / CL_ParamMap_MAXLOAD;
		struct ParamMapBucket * newdombuckets = calloc(1, newalloc);
		struct ParamMapBucket * newcodbuckets = calloc(1, newalloc);
		if ((newdombuckets == NULL) || (newcodbuckets == NULL)) {
			free(newdombuckets);
			free(newcodbuckets);
			return -1;
		}
		for (size_t i = 0; i <= bimap->sizemask; ++i) {
			if (bimap->dom_buckets[i].state == CL_ParamMap_BUCKET_OCCUPIED) {
				hilbert_pmap_store(newdombuckets, newsizemask, bimap->dom_buckets[i].entry,
						cl_hash32(bimap->dom_buckets[i].entry.pre));
			}
			if (bimap->cod_buckets[i].state == CL_ParamMap_BUCKET_OCCUPIED) {
				hilbert_pmap_store(newcodbuckets, newsizemask, bimap->cod_buckets[i].entry,
						cl_hash32(bimap->cod_buckets[i].entry.post));
			}
		}
		free(bimap->cod_buckets);
		free(bimap->dom_buckets);
		bimap->sizemask = newsizemask;
		bimap->threshold = newthreshold;
		bimap->dom_buckets = newdombuckets;
		bimap->cod_buckets = newcodbuckets;

		/* store mapping */
		hilbert_pmap_store(bimap->dom_buckets, bimap->sizemask, entry, pre_hash);
		hilbert_pmap_store(bimap->cod_buckets, bimap->sizemask, entry, post_hash);
		bimap->count = newcount;
		return 0;
	}

	/* store mapping */
	pre_cand->entry = entry;
	pre_cand->state = CL_ParamMap_BUCKET_OCCUPIED;
	post_cand->entry = entry;
	post_cand->state = CL_ParamMap_BUCKET_OCCUPIED;
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
 * 	The value being pointed to may not be altered (use <code>#hilbert_pmap_add()</code> for this purpose).
 * 	The returned pointer is guaranteed to be valid only until the next call to one of the bimap
 * 	functions with <code>bimap</code> as argument.
 * 	If no mapping for the given preimage exists, <code>NULL</code> is returned.
 */
static inline const HilbertHandle * hilbert_pmap_post(const ParamMap * bimap, HilbertHandle pre) {
	assert (bimap != NULL);

	struct ParamMapBucket * candidate = hilbert_pmap_find_pre(bimap->dom_buckets, bimap->sizemask, pre, cl_hash32(pre));
	if (candidate->state == CL_ParamMap_BUCKET_OCCUPIED)
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
 * The value being pointed to may not be altered (use <code>#hilbert_pmap_add()</code> for this purpose).
 * The returned pointer is guaranteed to be valid only until the next call to one of the bimap
 * functions with <code>bimap</code> as argument.
 * If no mapping for the given image exists, <code>NULL</code> is returned.
 */
static inline const HilbertHandle * hilbert_pmap_pre(const ParamMap * bimap, HilbertHandle post) {
	assert (bimap != NULL);

	struct ParamMapBucket * candidate = hilbert_pmap_find_post(bimap->cod_buckets, bimap->sizemask, post, cl_hash32(post));
	if (candidate->state == CL_ParamMap_BUCKET_OCCUPIED)
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
static inline ParamMapIterator hilbert_pmap_iterator_new(ParamMap * bimap) {
	assert (bimap != NULL);

	return (ParamMapIterator) { .bimap = bimap, .index = 0 };
}

/**
 * Checks whether an iterator has a next element.
 *
 * @return i Pointer to bimap iterator.
 *
 * @return If there is a next element in the iteration, <code>1</code> is returned.
 * 	Otherwise, <code>0</code> is returned.
 */
static inline int hilbert_pmap_iterator_hasnext(ParamMapIterator * i) {
	assert (i != NULL);

	for (; i->index <= i->bimap->sizemask; ++i->index) {
		if (i->bimap->dom_buckets[i->index].state == CL_ParamMap_BUCKET_OCCUPIED)
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
static inline struct ParamMapEntry hilbert_pmap_iterator_next(ParamMapIterator * i) {
	assert (i != NULL);

	for (;; ++i->index) {
		assert (i->index <= i->bimap->sizemask);
		if (i->bimap->dom_buckets[i->index].state == CL_ParamMap_BUCKET_OCCUPIED)
			return i->bimap->dom_buckets[i->index++].entry;
	}
}

#endif
