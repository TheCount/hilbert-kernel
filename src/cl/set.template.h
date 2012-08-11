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
 * @file set.template.h
 *
 * Replacements from set template:
 * - <code>SET</code>:
 *   Set typename.
 * - <code>VALUE_TYPE</code>:
 *   Type of the values stored.
 * - <code>PREFIX</code>:
 *   Function name prefix.
 * - <code>HASH</code>:
 *   Name of hash function to be used.
 * - <code>COMPARATOR</code>
 *   Name of value comparator.
 *   If COMPARATOR, the default (identity) comparator will be used.
 */

#ifndef CL_SET_H__
#define CL_SET_H__

#include<assert.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>

#include"hash.h"
#include"util.h"

/**
 * Comparator hack.
 * Makes the comparator all caps replacement optional.
 */
#define COMPARATOR 42
#define CL_A COMPA ## RATOR
#define CL_B COMPARATOR
#if ( CL_A == CL_B ) // comparator all caps was not replaced; valid expression by C99 6.10.1 clause 4, 3rd sentence.
#undef COMPARATOR
#define COMPARATOR( x, y ) ( ( x ) != ( y ) )
#else // comparator all caps was replaced
#undef COMPARATOR
#endif
#undef CL_A
#undef CL_B

/**
 * Initial number of buckets
 */
#define CL_SET_NUMBUCKETS 4

/**
 * Maximum load coefficient.
 */
#define CL_SET_MAXLOAD 4

/**
 * Bucket states.
 */
enum CLSETBucketState {
	CL_SET_BUCKET_EMPTY = 0,
	CL_SET_BUCKET_OCCUPIED,
	CL_SET_BUCKET_DELETED
};

/**
 * Bucket type.
 */
struct SETBucket {
	/**
	 * Bucket value.
	 */
	VALUE_TYPE value;
	/**
	 * Bucket state.
	 */
	enum CLSETBucketState state;
};

/**
 * Set structure.
 */
struct SET {
	/**
	 * Set item count.
	 */
	size_t count;

	/**
	 * Load threshold.
	 */
	size_t threshold;

	/**
	 * Total number of buckets, minus one
	 */
	size_t sizemask;

	/**
	 * Buckets
	 */
	struct SETBucket * buckets;
};

typedef struct SET SET;

/**
 * Initializes a new, empty set.
 *
 * @param set Pointer to a set.
 *
 * @return On success, 0 is returned.
 * 	On error, -1 is returned.
 */
static inline int PREFIX_init( SET * set ) WARN_UNUSED_RESULT;
static inline int PREFIX_init( SET * set ) {
	assert( set != NULL );

	set->count = 0;
	set->threshold = (CL_SET_MAXLOAD - 1) * CL_SET_NUMBUCKETS / CL_SET_MAXLOAD;
	set->sizemask = CL_SET_NUMBUCKETS - 1;

	set->buckets = calloc(CL_SET_NUMBUCKETS, sizeof( *set->buckets ) );
	if ( set->buckets == NULL ) {
		return -1;
	} else {
		return 0;
	}
}

/**
 * Creates a new, empty set.
 *
 * @return On success, a pointer to a new empty set is returned.
 * 	On error, <code>NULL</code> is returned.
 */
static inline SET * PREFIX_new(void) WARN_UNUSED_RESULT;
static inline SET * PREFIX_new(void) {
	SET * result;

	result = malloc(sizeof(*result));
	if (result == NULL)
		goto nosetmem;

	int rc = PREFIX_init( result );
	if ( rc != 0 ) {
		goto nobucketmem;
	}

	return result;

nobucketmem:
	free(result);
nosetmem:
	return NULL;
}

/**
 * Uninitializes a set.
 *
 * @param set Pointer to set.
 */
static inline void PREFIX_fini( SET * set ) {
	assert( set != NULL );

	free( set->buckets );
}

/**
 * Deletes a set.
 *
 * @param set Pointer to a set.
 */
static inline void PREFIX_del(SET * set) {
	assert (set != NULL);

	PREFIX_fini( set );
	free(set);
}

/**
 * Clones a set.
 *
 * @param set Pointer to set that is to be cloned.
 *
 * @return On success, a pointer to a new set is returned, containing precisely the elements of the old set.
 * 	On error, <code>NULL</code> is returned.
 */
static inline SET * PREFIX_clone(const SET * set) WARN_UNUSED_RESULT;
static inline SET * PREFIX_clone(const SET * set) {
	assert (set != NULL);

	SET * clone;
	clone = malloc(sizeof(*clone));
	if (clone == NULL)
		return NULL;

	clone->count = set->count;
	clone->threshold = set->threshold;
	clone->sizemask = set->sizemask;

	size_t bucketalloc = (clone->sizemask + 1) * sizeof(*clone->buckets);
	clone->buckets = malloc(bucketalloc);
	if (clone->buckets == NULL) {
		free(clone);
		return NULL;
	}
	memcpy(clone->buckets, set->buckets, bucketalloc);

	return clone;
}

/**
 * Stores a value in the correct bucket (private).
 *
 * @param buckets Pointer to an array of buckets. At least one bucket in the array must be unoccupied.
 * @param sizemask Size of the array pointed to by <code>buckets</code>, minus one.
 * @param value Value to be stored. The value must not be present in any of the buckets.
 * @param hash Hash code of <code>value</code>.
 */
static inline void PREFIX_store(struct SETBucket * buckets, size_t sizemask, VALUE_TYPE value, size_t hash) {
	assert (buckets != NULL);
	assert (sizemask > 0);
	assert (sizemask < SIZE_MAX);
	assert ((sizemask & (sizemask + 1)) == 0);

	for (size_t i = hash & sizemask; 1; i = (i + 1) & sizemask) {
		if (buckets[i].state != CL_SET_BUCKET_OCCUPIED) {
			buckets[i].value = value;
			buckets[i].state = CL_SET_BUCKET_OCCUPIED;
			break;
		}
	}
}

/**
 * Finds a candidate bucket (private).
 *
 * @param buckets Pointer to an array of buckets.
 * @param sizemask Size of the array pointed to by <code>buckets</code>, minus one.
 * 	The array size must be a power of two.
 * @param value Value to be searched for/to be stored in the candidate bucket.
 * @param hash Hash code of <code>value</code>.
 *
 * @return A pointer to an element in the <code>buckets</code> array is returned.
 * 	If the element is occupied, it contains <code>value</code>.
 * 	Otherwise, the element is suitable for storing <code>value</code>.
 */
static inline struct SETBucket * PREFIX_find(struct SETBucket * buckets, size_t sizemask, const VALUE_TYPE value, size_t hash) {
	assert (buckets != NULL);
	assert (sizemask > 0);
	assert (sizemask < SIZE_MAX);
	assert ((sizemask & (sizemask + 1)) == 0);

	int deletedfound = 0;
	struct SETBucket * firstdeleted;
	size_t count = 0;
	for (size_t i = hash & sizemask; count <= sizemask; i = (i + 1) & sizemask, ++count) {
		switch (buckets[i].state) {
			case CL_SET_BUCKET_EMPTY:
				if (deletedfound) {
					return firstdeleted;
				} else {
					return &buckets[i];
				}
				break;
			case CL_SET_BUCKET_OCCUPIED:
				if ( COMPARATOR( buckets[i].value, value ) == 0 )
					return &buckets[i];
				break;
			case CL_SET_BUCKET_DELETED:
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
 * Adds an element to a set.
 * 
 * @param set Pointer to a set to which an element is to be added.
 * @param value Value to be added to the set.
 *
 * @return On success, <code>0</code> is returned.
 * 	On error, <code>-1</code> is returned.
 */
static inline int PREFIX_add(SET * set, VALUE_TYPE value) WARN_UNUSED_RESULT;
static inline int PREFIX_add(SET * set, VALUE_TYPE value) {
	assert (set != NULL);

	size_t hash = HASH(value);
	struct SETBucket * candidate = PREFIX_find(set->buckets, set->sizemask, value, hash);
	if (candidate->state == CL_SET_BUCKET_OCCUPIED) /* already in set */
		return 0;

	assert (set->count < SIZE_MAX);

	size_t newcount = set->count + 1;
	if (newcount > set->threshold) {
		/* rebuild table */
		size_t oldalloc = (set->sizemask + 1) * sizeof(*set->buckets);
		size_t newalloc = 2 * oldalloc;
		if (newalloc <= oldalloc)
			return -1;
		size_t newsize = newalloc / sizeof(*set->buckets);
		size_t newsizemask = newsize - 1;
		size_t newthreshold = (CL_SET_MAXLOAD - 1) * newsize / CL_SET_MAXLOAD;
		struct SETBucket * newbuckets = calloc(1, newalloc);
		if (newbuckets == NULL)
			return -1;
		for (size_t i = 0; i <= set->sizemask; ++i) {
			if (set->buckets[i].state == CL_SET_BUCKET_OCCUPIED)
				PREFIX_store(newbuckets, newsizemask, set->buckets[i].value, HASH(set->buckets[i].value));
		}
		free (set->buckets);
		set->sizemask = newsizemask;
		set->threshold = newthreshold;
		set->buckets = newbuckets;

		/* store value */
		PREFIX_store(set->buckets, set->sizemask, value, hash);
		set->count = newcount;
		return 0;
	}

	/* store value */
	candidate->value = value;
	candidate->state = CL_SET_BUCKET_OCCUPIED;
	set->count = newcount;
	return 0;
}

/**
 * Adds all values from one set to another.
 *
 * @param dest Pointer to destination set.
 * @param src Pointer to source set. Must be distinct from destrination set.
 *
 * @return On success, <code>0</code> is returned.
 * 	On error, <code>-1</code> is returned.
 */
static inline int PREFIX_addall(SET * restrict dest, const SET * restrict src) WARN_UNUSED_RESULT;
static inline int PREFIX_addall(SET * restrict dest, const SET * restrict src) {
	assert (dest != NULL);
	assert (src != NULL);

	int rc = -1;

	/* first, create a backup set in case something goes wrong */
	SET backup = *dest;
	size_t allocsize = (backup.sizemask + 1) * sizeof(*backup.buckets);
	backup.buckets = malloc(allocsize);
	if (backup.buckets == NULL)
		goto backupallocfail;
	memcpy(backup.buckets, dest->buckets, allocsize);

	/* Now add new elements */
	for (size_t i = 0; i <= src->sizemask; ++i) {
		if (src->buckets[i].state == CL_SET_BUCKET_OCCUPIED) {
			if (PREFIX_add(dest, src->buckets[i].value) != 0) {
				/* restore backup and bailout */
				free(dest->buckets);
				*dest = backup;
				goto addfail;
			}
		}
	}

	free(backup.buckets);
	rc = 0;

addfail:
backupallocfail:
	return rc;
}

/**
 * Removes an element from a set.
 *
 * @param set Pointer to a set.
 * @param value Value to be removed.
 *
 * @return If <code>value</code> was present in the set pointed to by <code>set</code>, <code>1</code> is returned.
 * 	Otherwise, <code>0</code> is returned.
 */
static inline int PREFIX_remove(SET * set, const VALUE_TYPE value) {
	assert (set != NULL);

	struct SETBucket * candidate = PREFIX_find(set->buckets, set->sizemask, value, HASH(value));
	if (candidate->state != CL_SET_BUCKET_OCCUPIED)
		return 0;

	candidate->state = CL_SET_BUCKET_DELETED;
	assert (set->count > 0);
	--set->count;

	return 1;
}

/**
 * Obtains an element from a set.
 *
 * @param set Pointer to a set.
 * @param value Value euqal to the value to be obtained.
 *
 * @return An entry equal to <code>value</code> is returned from the set.
 * 	If no such entry exists, the behaviour is undefined.
 */
static inline VALUE_TYPE PREFIX_get( SET * set, const VALUE_TYPE value ) {
	assert( set != NULL );

	struct SETBucket * candidate = PREFIX_find(set->buckets, set->sizemask, value, HASH(value));
	assert( candidate->state == CL_SET_BUCKET_OCCUPIED );

	return candidate->value;
}

/**
 * Clears a set.
 *
 * @param set Pointer to a set.
 */
static inline void PREFIX_clear(SET * set) {
	assert (set != NULL);

	set->count = 0;
	memset(set->buckets, 0, (set->sizemask + 1) * sizeof(*set->buckets));
}

/**
 * Checks whether a value is present in a set.
 *
 * @param set Pointer to a set.
 * @param value Value to be looked for.
 *
 * @return If <code>value</code> is present in the set pointed to by <code>set</code>, <code>1</code> is returned.
 * 	Otherwise, <code>0</code> is returned.
 */
static inline int PREFIX_contains(const SET * set, const VALUE_TYPE value) {
	assert (set != NULL);

	return (PREFIX_find(set->buckets, set->sizemask, value, HASH(value))->state == CL_SET_BUCKET_OCCUPIED);
}

/**
 * Returns the number of elements in a set.
 *
 * @param set Pointer to a set.
 *
 * @return The number of elements in the set is returned.
 */
static inline size_t PREFIX_count(const SET * set) {
	assert (set != NULL);

	return set->count;
}

/**
 * Starts an iteration over a set.
 *
 * @param set Pointer to a set.
 *
 * @return An opaque iterator handle pointing to the first element in the
 * 	iteration is returned. If the specified set is empty, NULL is
 * 	returned.
 */
static inline void * PREFIX_iterator_start( const SET * set ) WARN_UNUSED_RESULT;
static inline void * PREFIX_iterator_start( const SET * set ) {
	assert( set != NULL );

	for ( size_t i = 0; i <= set->sizemask; ++i ) {
		if ( set->buckets[i].state == CL_SET_BUCKET_OCCUPIED ) {
			return &set->buckets[i];
		}
	}

	return NULL;
}

/**
 * Returns the current value in a set iteration.
 *
 * @param set Pointer to a set.
 * @param iter Iteration handle belonging to the specified set.
 *
 * @return The value the iteration handle is pointing to is returned.
 */
static inline VALUE_TYPE PREFIX_iterator_get( const SET * set, void * iter ) {
	assert( set != NULL );
	assert( iter != NULL );

	struct SETBucket * current = iter;
	assert( ( set->buckets <= current ) && ( current <= set->buckets + set->sizemask ) );

	return current->value;
}

/**
 * Continues an iteration over a set.
 *
 * @param set Pointer to a set.
 * @param iter Iteration handle belonging to the specified set.
 *
 * @return An iteration handle pointing to the next element in the iteration
 * 	is returned. If the current element is the last element in the
 * 	iteration, NULL is returned.
 */
static inline void * PREFIX_iterator_next( const SET * set, void * iter ) WARN_UNUSED_RESULT;
static inline void * PREFIX_iterator_next( const SET * set, void * iter ) {
	assert( set != NULL );
	assert( iter != NULL );

	struct SETBucket * current = iter;
	assert( ( set->buckets <= current ) && ( current <= set->buckets + set->sizemask ) );

	for ( size_t i = current - set->buckets + 1; i <= set->sizemask; ++i ) {
		if ( set->buckets[i].state == CL_SET_BUCKET_OCCUPIED ) {
			return &set->buckets[i];
		}
	}

	return NULL;
}

/* End of comparator hack. */
#undef COMPARATOR

#endif
