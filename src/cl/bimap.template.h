/*
 *  Template container library.
 *  Copyright © 2011, 2012 Alexander Klauer
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
 * @file bimap.template.h
 *
 * Replacements from bimap template:
 * - <code>BIMAP</code>:
 *   Bimap typename.
 * - <code>DOM_TYPE</code>:
 *   Type of the bimap's domain.
 * - <code>COD_TYPE</code>:
 *   Type of the bimap's codomain.
 * - <code>ENTRY_TYPE</code>:
 *   Type of the bimap entries.
 * - <code>PREFIX</code>:
 *   Function name prefix.
 * - <code>DOM_HASH</code>:
 *   Name of domain hash function.
 * - <code>COD_HASH</code>:
 *   Name of codomain hash function.
 * - <code>DOM_COMPARATOR</code>
 *   Name of domain element comparator (default: identity).
 * - <code>COD_COMPARATOR</code>
 *   Name of codomain element comparator (default: identity).
 */

#ifndef CL_BIMAP_H__
#define CL_BIMAP_H__

#include<assert.h>
#include<stdint.h>
#include<stdlib.h>

#include"hash.h"
#include"util.h"

/**
 * Initial number of buckets.
 */
#define CL_BIMAP_NUMBUCKETS 4

/**
 * Maximum load coefficient.
 */
#define CL_BIMAP_MAXLOAD 4

/**
 * Comparator hack.
 */
#define DOM_COMPARATOR 42
#define CL_A DOM_COMPA ## RATOR
#define CL_B DOM_COMPARATOR
#if ( CL_A == CL_B )
#undef DOM_COMPARATOR
#define DOM_COMPARATOR( x, y ) ( ( x ) != ( y ) )
#else
#undef DOM_COMPARATOR
#endif
#undef CL_A
#undef CL_B
#define COD_COMPARATOR 42
#define CL_A COD_COMPA ## RATOR
#define CL_B COD_COMPARATOR
#if ( CL_A == CL_B )
#undef COD_COMPARATOR
#define COD_COMPARATOR( x, y ) ( ( x ) != ( y ) )
#else
#undef COD_COMPARATOR
#endif
#undef CL_A
#undef CL_B

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
 * Initialized an empty bimap.
 *
 * @param bimap Pointer to a bimap.
 *
 * @return On success, 0 is returned.
 * 	On error, -1 is returned.
 */
static inline int PREFIX_init( struct BIMAP * bimap ) WARN_UNUSED_RESULT;
static inline int PREFIX_init( struct BIMAP * bimap ) {
	assert( bimap != NULL );

	bimap->count = 0;
	bimap->threshold = (CL_BIMAP_MAXLOAD - 1) * CL_BIMAP_NUMBUCKETS / CL_BIMAP_MAXLOAD;
	bimap->sizemask = CL_BIMAP_NUMBUCKETS - 1;

	bimap->dom_buckets = calloc(CL_BIMAP_NUMBUCKETS, sizeof(*bimap->dom_buckets));
	if (bimap->dom_buckets == NULL)
		goto nodommem;

	bimap->cod_buckets = calloc(CL_BIMAP_NUMBUCKETS, sizeof(*bimap->dom_buckets));
	if (bimap->cod_buckets == NULL)
		goto nocodmem;

	return 0;

nocodmem:
	free( bimap->dom_buckets );
nodommem:
	return -1;
}

/**
 * Creates a new, empty bimap.
 *
 * @return On success, a pointer to a new, empty bimap is returned.
 * 	On error, <code>NULL</code> is returned.
 */
static inline BIMAP * PREFIX_new(void) WARN_UNUSED_RESULT;
static inline BIMAP * PREFIX_new(void) {
	BIMAP * result;

	result = malloc(sizeof(*result));
	if (result == NULL)
		goto nomapmem;

	int rc = PREFIX_init( result );
	if ( rc != 0 ) {
		goto noinit;
	}

	return result;

noinit:
	free(result);
nomapmem:
	return NULL;
}

/**
 * Uninitializes a bimap.
 *
 * @param bimap Pointer to a bimap.
 */
static inline void PREFIX_fini( struct BIMAP * bimap ) {
	assert( bimap != NULL );

	free( bimap->cod_buckets );
	free( bimap->dom_buckets );
}

/**
 * Frees a bimap.
 *
 * @param bimap Pointer to a bimap.
 */
static inline void PREFIX_del(BIMAP * bimap) {
	assert (bimap != NULL);

	PREFIX_fini( bimap );
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
				if ( DOM_COMPARATOR( buckets[i].entry.pre, pre ) == 0 )
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
				if ( COD_COMPARATOR( buckets[i].entry.post, post ) == 0 )
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
static inline int PREFIX_add(BIMAP * bimap, DOM_TYPE pre, COD_TYPE post) WARN_UNUSED_RESULT;
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
 * Starts an iteration over a bimap.
 *
 * @param bimap Pointer to a bimap.
 *
 * @return An opaque iteration handle for the map pinted to by bimap is
 * 	returned. If the map is empty, NULL is returned.
 */
static inline void * PREFIX_iterator_start( const BIMAP * bimap ) WARN_UNUSED_RESULT;
static inline void * PREFIX_iterator_start( const BIMAP * bimap ) {
	assert ( bimap != NULL );

	for ( size_t i = 0; i <= bimap->sizemask; ++i ) {
		if ( bimap->dom_buckets[i].state == CL_BIMAP_BUCKET_OCCUPIED ) {
			return &bimap->dom_buckets[i];
		}
	}

	return NULL;
}

/**
 * Returns the current entry in an iteration.
 *
 * @param bimap Pointer to bimap.
 * @param iter Iteration handle for the specified bimap.
 *
 * @return The current entry the iteration handle is pointing to is returned.
 */
static inline struct ENTRY_TYPE PREFIX_iterator_get( const BIMAP * bimap, void * iter ) {
	assert( bimap != NULL );
	assert( iter != NULL );

	struct BIMAPBucket * current = iter;
	assert( ( bimap->dom_buckets <= current ) && ( current <= bimap->dom_buckets + bimap->sizemask ) );

	return current->entry;
}

/**
 * Returns the next element in a bimap iteration.
 *
 * @param bimap Pointer to bimap.
 * @param iter Iteration handle for the specified bimap.
 *
 * @return An iteration handle for the next element in the iteration is
 * 	returned, or NULL if the current element was the last element in the
 * 	iteration.
 */
static inline void * PREFIX_iterator_next( const BIMAP * bimap, void * iter ) WARN_UNUSED_RESULT;
static inline void * PREFIX_iterator_next( const BIMAP * bimap, void * iter ) {
	assert( bimap != NULL );
	assert( iter != NULL );

	struct BIMAPBucket * current = iter;
	assert( ( bimap->dom_buckets <= current ) && ( current <= bimap->dom_buckets + bimap->sizemask ) );

	for ( size_t i = current - bimap->dom_buckets + 1; i <= bimap->sizemask; ++i ) {
		if ( bimap->dom_buckets[i].state == CL_BIMAP_BUCKET_OCCUPIED ) {
			return &bimap->dom_buckets[i];
		}
	}

	return NULL;
}

/* End of comparator hack */
#undef DOM_COMPARATOR
#undef COD_COMPARATOR

#endif
