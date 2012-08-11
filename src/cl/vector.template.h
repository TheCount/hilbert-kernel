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
 * @file vector.template.h
 *
 * Replacements from vector container template:
 * - <code>VECTOR</code>:
 *   Vector type name,
 * - <code>VALUE_TYPE</code>:
 *   Type name of the value type,
 * - <code>PREFIX</code>:
 *   Prefix of function names.
 * - <code>COMPARATOR</code>:
 *   Comparator function (if sorting is needed).
 */

#ifndef CL_VECTOR_H__
#define CL_VECTOR_H__

#include<assert.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>

#include"util.h"

/**
 * Comparator hack.
 * If a replacement for a comparator is provided, the vector becomes sortable.
 */
#define COMPARATOR 42
#define CL_A COMPA ## RATOR
#define CL_B COMPARATOR
#if ( CL_A == CL_B ) // no comparator provided
#undef COMPARATOR
#else // comparator provided
#undef COMPARATOR
#define CL_VECTOR_IS_SORTABLE
#endif
#undef CL_A
#undef CL_B

/**
 * Vector structure.
 */
struct VECTOR {
	/**
	 * Vector item count.
	 */
	size_t count;

	/**
	 * current maximum number of items.
	 */
	size_t size;

	/**
	 * Vector data array.
	 */
	VALUE_TYPE * data;
};

typedef struct VECTOR VECTOR;

/**
 * Initializes a new vector with the specified minimal size.
 *
 * @param vector Pointer to vector.
 * @param minsize Minimal size.
 *
 * @return On success, 0 is returned.
 * 	On error, -1 is returned.
 */
static inline int PREFIX_init2( VECTOR * vector, size_t minsize ) WARN_UNUSED_RESULT;
static inline int PREFIX_init2( VECTOR * vector, size_t minsize ) {
	assert( vector != NULL );

	vector->count = 0;
	vector->size = roundup2( minsize );
	assert( vector->size > 0 );
	vector->data = malloc( vector->size * sizeof( *vector->data ) );
	if ( vector->data == NULL ) {
		return -1;
	} else {
		return 0;
	}
}

/**
 * Initializes a new vector.
 *
 * @param vector Pointer to vector.
 *
 * @return On success, 0 is returned.
 * 	On error, -1 is returned.
 */
static inline int PREFIX_init( VECTOR * vector ) WARN_UNUSED_RESULT;
static inline int PREFIX_init( VECTOR * vector ) {
	return PREFIX_init2( vector, 0 );
}

/**
 * Creates a new, empty vector with the specified minimal size.
 *
 * @param minsize Minimal size.
 *
 * @return On success, a pointer to a new, empty vector is returned.
 * 	On error, <code>NULL</code> is returned.
 */
static inline VECTOR * PREFIX_new2( size_t minsize ) WARN_UNUSED_RESULT;
static inline VECTOR * PREFIX_new2( size_t minsize ) {
	VECTOR * result;

	result = malloc(sizeof(*result));
	if (result == NULL)
		return NULL;

	int rc = PREFIX_init2( result, minsize );
	if ( rc != 0 ) {
		free( result );
		return NULL;
	}

	return result;
}

/**
 * Creates a new, empty vector.
 *
 * @return On success, a pointer to a new, empty vector is returned.
 * 	On error, <code>NULL</code> is returned.
 */
static inline VECTOR * PREFIX_new(void) WARN_UNUSED_RESULT;
static inline VECTOR * PREFIX_new(void) {
	return PREFIX_new2( 0 );
}

/**
 * Uninitializes a vector.
 *
 * @param vector Pointer to vector.
 */
static inline void PREFIX_fini( VECTOR * vector ) {
	assert( vector != NULL );

	free( vector->data );
}

/**
 * Dismantles a vector, destroying it, but returning its contents.
 *
 * @param vector Pointer to vector.
 *
 * @return A pointer to an array containing the elements of the specified
 * 	vector is returned. The vector itself is destroyed as if by
 * 	#PREFIX_fini().
 * 	The returned array must be freed by the caller.
 */
static inline VALUE_TYPE * PREFIX_dismantle( VECTOR * vector ) WARN_UNUSED_RESULT;
static inline VALUE_TYPE * PREFIX_dismantle( VECTOR * vector ) {
	assert( vector != NULL );

	VALUE_TYPE * result = realloc( vector->data, vector->count * sizeof( *vector->data ) );
	if ( ( vector->count > 0 ) && ( result == NULL ) ) {
		result = vector->data;
	}

	return result;
}

/**
 * Deletes a vector.
 *
 * @param vector pointer to the vector which is to be deleted.
 */
static inline void PREFIX_del(VECTOR * vector) {
	assert (vector != NULL);

	PREFIX_fini( vector );
	free(vector);
}

/**
 * Adds an element to the end of a vector.
 *
 * @param vector Pointer to the vector to which an element is to be added.
 * @param elt Element to be added.
 *
 * @return On success, <code>0</code> is returned.
 * 	On error, <code>-1</code> is returned.
 */
static inline int PREFIX_pushback(VECTOR * vector, VALUE_TYPE elt) WARN_UNUSED_RESULT;
static inline int PREFIX_pushback(VECTOR * vector, VALUE_TYPE elt) {
	assert (vector != NULL);
	assert (vector->count < SIZE_MAX);

	size_t newcount = vector->count + 1;
	if (newcount > vector->size) {
		/* grow vector */
		size_t oldalloc = vector->size * sizeof(*vector->data);
		size_t newalloc = 2 * oldalloc;
		if (newalloc <= oldalloc)
			return -1;
		size_t newsize = newalloc / sizeof(*vector->data);
		VALUE_TYPE * newdata = realloc(vector->data, newalloc);
		if (newdata == NULL)
			return -1;
		vector->size = newsize;
		vector->data = newdata;
	}
	vector->data[vector->count] = elt;
	vector->count = newcount;
	return 0;
}

/**
 * Removes an element from the end of a vector.
 *
 * @param vector Pointer to the vector from which an element is to be removed.
 * 	If the vector does not have any elements, the behaviour is undefined.
 *
 * @return The removed element is returned.
 */
static inline VALUE_TYPE PREFIX_popback(VECTOR * vector) {
	assert (vector != NULL);
	assert (vector->count > 0);

	return vector->data[--vector->count];
}

/**
 * Returns the first element in a vector.
 *
 * @param vector Pointer to a vector.
 *
 * @return The first element in the specified vector is returned.
 * 	If the vector does not have any elements, the behaviour is undefined.
 */
static inline VALUE_TYPE PREFIX_first( const VECTOR * vector ) {
	assert( vector != NULL );
	assert( vector->count > 0 );

	return vector->data[0];
}

/**
 * Returns the last element in a vector.
 *
 * @param vector Pointer to a vector.
 *
 * @return The last element in the specified vector is returned.
 * 	If the vector does not have any elements, he behaviour is undefined.
 */
static inline VALUE_TYPE PREFIX_last( const VECTOR * vector ) {
	assert( vector != NULL );
	assert( vector->count > 0 );

	return vector->data[vector->count - 1];
}

/**
 * Copies a vector, appending its contents to the end of another vector.
 *
 * @param dest Pointer to the vector to whose end elements are to be appended.
 * @param src Pointer to the vector whose elements are to be appended to the
 * 	end of the vector pointed to by <code>dest</code>.
 *
 * @return On success, 0 is returned.
 * 	On error, -1 is returned.
 */
static inline int PREFIX_append( VECTOR * restrict dest, const VECTOR * restrict src ) {
	assert( dest != NULL );
	assert( src != NULL );

	size_t newcount = dest->count + src->count;
	size_t newsize = roundup2( newcount );
	if ( newsize < dest->size ) {
		return -1;
	} else if ( newsize > dest->size ) {
		size_t newalloc = newsize * sizeof( VALUE_TYPE );
		if ( newalloc <= sizeof( VALUE_TYPE ) ) {
			return -1;
		}
		VALUE_TYPE * newdata = realloc( dest->data, newalloc );
		if ( newdata == NULL ) {
			return -1;
		}
		dest->size = newsize;
		dest->data = newdata;
	}
	memcpy( dest->data + dest->count, src->data, src->count * sizeof( VALUE_TYPE ) );
	dest->count = newcount;

	return 0;
}

/**
 * Copies a vector, returning its contents as an array.
 *
 * @param vector Pointer to a vector.
 *
 * @return On success, a pointer to an array containing a copy of the contents
 * 	of the specified vector is returned. If the vector is empty, the
 * 	returned pointer may be a null pointer.
 * 	On error, NULL is returned.
 */
static inline VALUE_TYPE * PREFIX_toarray( const VECTOR * vector ) {
	assert( vector != NULL );

	VALUE_TYPE * result = malloc( vector->count * sizeof( VALUE_TYPE ) );
	if ( result == NULL ) {
		return NULL;
	}
	memcpy( result, vector->data, vector->count * sizeof( VALUE_TYPE ) );

	return result;
}

/**
 * Downsizes a vector.
 *
 * @param vector Pointer to the vector which is to be downsized.
 * @param newcount New vector size.
 * 	It is an error if the new vector size is larger than the current vector size.
 *
 * @return On success, <code>0</code> is returned.
 * 	On error, <code>-1</code> is returned.
 */
static inline int PREFIX_downsize(VECTOR * vector, size_t newcount) {
	assert (vector != NULL);
	if (newcount > vector->count)
		return -1;
	vector->count = newcount;
	// FIXME: Shrink allocated space?
	return 0;
}

/**
 * Returns the number of elements in a vector.
 *
 * @param vector Pointer to the vector whose number of elements is to be returned.
 *
 * @return Number of elements in the vector.
 */
static inline size_t PREFIX_count(const VECTOR * vector) {
	assert (vector != NULL);

	return vector->count;
}

/**
 * Returns an element from a vector.
 *
 * @param vector Pointer to the vector from which an element is to be returned.
 * @param index Index of the element to be returned.
 * 	If the index is out of bounds, the behaviour is undefined.
 *
 * @return The element specified by <code>index</code> is returned.
 */
static inline VALUE_TYPE PREFIX_get(const VECTOR * vector, size_t index) {
	assert (vector != NULL);
	assert (index < vector->count);

	return vector->data[index];
}

/**
 * Returns a pointer to an element in a vector.
 * The element can be altered in-place using the pointer.
 * The pointer is only valid as long as no elements are added to or removed from the vector.
 *
 * @param vector Pointer to the vector from which a pointer to one of its elements is to be returned.
 * @param index Index of the element.
 * 	If the index is out of bounds, the behavior is undefined.
 *
 * @return A pointer to the element specified by <code>index</code> is returned.
 */
static inline VALUE_TYPE * PREFIX_getp( struct VECTOR * vector, size_t index ) {
	assert( vector != NULL );
	assert( index < vector->count );

	return &vector->data[index];
}

/**
 * Sets the current value of an element at a position.
 *
 * @param vector Pointer to a vector.
 * @param index Index of the element.
 * 	If the index is out of bounds, the behavior is undefined.
 * @param value New value.
 */
static inline void PREFIX_set( struct VECTOR * vector, size_t index, VALUE_TYPE value ) {
	assert( vector != NULL );
	assert( index < vector->count );

	vector->data[index] = value;
}

/**
 * Starts an iteration over a vector.
 * During an iteration, no new elements may be added to the vector.
 *
 * @param vector Pointer to a vector.
 *
 * @return An opaque iteration handle pointing to the first element of the
 * 	specified vector is returned. If the vector is empty, NULL is
 * 	returned.
 */
static inline void * PREFIX_iterator_start( const VECTOR * vector ) WARN_UNUSED_RESULT;
static inline void * PREFIX_iterator_start( const VECTOR * vector ) {
	assert( vector != NULL );

	if ( vector->count == 0 ) {
		return NULL;
	} else {
		return vector->data;
	}
}

/**
 * Starts a reverse iteration over a vector.
 * During an iteration, no new elements may be added to the vector.
 *
 * @param vector Pointer to a vector.
 *
 * @return An opaque iteration handle pointing to the last element of the
 * 	specified vector is returned. If the vector is empty, NULL is
 * 	returned.
 */
static inline void * PREFIX_iterator_end( const VECTOR * vector ) WARN_UNUSED_RESULT;
static inline void * PREFIX_iterator_end( const VECTOR * vector ) {
	assert( vector != NULL );

	if ( vector->count == 0 ) {
		return NULL;
	} else {
		return &vector->data[vector->count - 1];
	}
}


/**
 * Returns the current value in an iteration over a vector.
 *
 * @param vector Pointer to a vector.
 * @param iter Iteration handle belonging to the specified vector.
 *
 * @return The current value in the iteration is returned.
 */
static inline VALUE_TYPE PREFIX_iterator_get( const VECTOR * vector, void * iter ) {
	assert( vector != NULL );
	assert( iter != NULL );

	VALUE_TYPE * current = iter;
	assert( ( vector->data <= current ) && ( current < vector->data + vector->count ) );

	return *current;
}

/**
 * Returns a pointer to the current value in an iteration over a vector.
 *
 * @param vector Pointer to a vector.
 * @param iter Iteration handle belonging to the specified vector.
 *
 * @return A pointer to the current value in the iteration is returned.
 */
static inline VALUE_TYPE * PREFIX_iterator_getp( const VECTOR * vector, void * iter ) {
	assert( vector != NULL );
	assert( iter != NULL );

	VALUE_TYPE * current = iter;
	assert( ( vector->data <= current ) && ( current < vector->data + vector->count ) );

	return current;
}

/**
 * Continues an iteration over a vector.
 *
 * @param vector Pointer to a vector.
 * @param iter Iteration handle belonging to the specified vector.
 *
 * @return An iteration handle pointing to the next element in the iteration
 * 	is returned. If the current element is the last element in the
 * 	iteration, NULL is returned.
 */
static inline void * PREFIX_iterator_next( const VECTOR * vector, void * iter ) WARN_UNUSED_RESULT;
static inline void * PREFIX_iterator_next( const VECTOR * vector, void * iter ) {
	assert( vector != NULL );
	assert( iter != NULL );

	VALUE_TYPE * current = iter;
	assert( ( vector->data <= current ) && ( current < vector->data + vector->count ) );

	++current;
	if ( current == vector->data + vector->count ) {
		return NULL;
	} else {
		return current;
	}
}
 
/**
 * Continues a reverse iteration over a vector.
 *
 * @param vector Pointer to a vector.
 * @param iter Iteration handle belonging to the specified vector.
 *
 * @return An iteration handle pointing to the previous element in the
 * 	iteration is returned. If the current element is the first element in
 * 	the iteration, NULL is returned.
 */
static inline void * PREFIX_iterator_previous( const VECTOR * vector, void * iter ) WARN_UNUSED_RESULT;
static inline void * PREFIX_iterator_previous( const VECTOR * vector, void * iter ) {
	assert( vector != NULL );
	assert( iter != NULL );

	VALUE_TYPE * current = iter;
	assert( ( vector->data <= current ) && ( current < vector->data + vector->count ) );

	if ( current == vector->data ) {
		return NULL;
	} else {
		return current - 1;
	}
}

#ifdef CL_VECTOR_IS_SORTABLE // sort implementation, see http://www.keithschwarz.com/smoothsort/ for details

#ifndef CL_LEONARDO_NUMBERS_DEFINED__
#define CL_LEONARDO_NUMBERS_DEFINED__

/**
 * Leonardo number sequence, up to 64 elements.
 */
static const size_t CL_LEONARNO_NUMBERS[] = {
	/* Leonardo numbers below 2**16 */
	1u, 1u, 3u, 5u, 9u, 15u, 25u, 41u, 67u, 109u, 177u, 287u, 465u, 753u, 1219u, 1973u, 3193u, 5167u, 8361u, 13529u, 21891u, 35421u, 57313u,
#define CL_LEONARDO_MAX 57313u
#if ( SIZE_MAX > 0xFFFF )
	/* Further Leonardo numbers below 2**24 */
	92735u, 150049u, 242785u, 392835u, 635621u, 1028457u, 1664079u, 2692537u, 4356617u, 7049155u, 11405773u,
#undef CL_LEONARDO_MAX
#define CL_LEONARDO_MAX 11405773u
#endif
#if ( SIZE_MAX > 0xFFFFFF )
	/* Further Leonardo numbers below 2**32 */
	18454929u, 29860703u, 48315633u, 78176337u, 126491971u, 204668309u, 331160281u, 535828591u, 866988873u, 1402817465u, 2269806339u, 3672623805u,
#undef CL_LEONARDO_MAX
#define CL_LEONARDO_MAX 3672623805u
#endif
#if ( SIZE_MAX > 0xFFFFFFFF )
	/* Further Leonardo numbers */
	5942430145u, 9615053951u, 15557484097u, 25172538049u, 40730022147u, 65902560197u, 106632582345u, 172535142543u, 279167724889u, 451702867433u, 730870592323u,
	1182573459757u, 1913444052081u, 3096017511839u, 5009461563921u, 8105479075761u, 13114940639683u, 21220419715445u,
#undef CL_LEONARDO_MAX
#define CL_LEONARDO_MAX 21220419715445u
#endif
};

/**
 * Leonardo bitvector.
 */
#if ( CL_LEONARDO_MAX <= 4356617u )
typedef uint_fast32_t CLLeonardoBitvector;
#else
typedef uint_fast64_t CLLeonardoBitvector;
#endif

#endif // Leonardo numbers

/**
 * Rebalances a Leonardo heap (private).
 *
 * @param vector Pointer to a vector.
 * @param shift Heap shift.
 * @param root Position of the (right-aligned) heap root.
 */
static inline void PREFIX_lheap_rebalance( VECTOR * vector, int shift, size_t root ) {
	assert( vector != NULL );
	assert( root < vector->count );

	while ( shift >= 2 ) {
		assert( root > 0 );
		assert( root - 1 >= CL_LEONARNO_NUMBERS[shift - 2] );
		shift -= 2;
		size_t second = root - 1;
		size_t first = second - CL_LEONARNO_NUMBERS[shift];

		size_t next;
		if ( COMPARATOR( vector->data[first], vector->data[second] ) > 0 ) {
			next = first;
			++shift;
		} else {
			next = second;
		}
		if ( COMPARATOR( vector->data[root], vector->data[next] ) > 0 ) {
			return;
		}

		VALUE_TYPE tmp = vector->data[root];
		vector->data[root] = vector->data[next];
		vector->data[next] = tmp;
		root = next;
	}
}

/**
 * Rectifies a Leonardo heap.
 *
 * @param vector Pointer to vector.
 * @param lv Heap shape.
 * @param shift Heap shift.
 * @param end Position where the new heap root is going to be.
 */
static inline void PREFIX_lheap_rectify( VECTOR * vector, CLLeonardoBitvector lv, int shift, size_t end ) {
	assert( vector != NULL );
	assert( end < vector->count );

	size_t i, oshift, previous;
	for ( i = end, oshift = shift; i != CL_LEONARNO_NUMBERS[oshift] - 1; oshift = shift, i = previous ) {
		size_t larger = i;
		/* Store actual heap root in larger */
		if ( shift >= 2 ) {
			assert( i > 0 );
			assert( i - 1 >= CL_LEONARNO_NUMBERS[shift - 2] );
			size_t second = i - 1;
			size_t first = second - CL_LEONARNO_NUMBERS[shift - 2];
			if ( COMPARATOR( vector->data[first], vector->data[second] ) > 0 ) {
				larger = first;
			} else {
				larger = second;
			}
			if ( COMPARATOR( vector->data[i], vector->data[larger] ) > 0 ) {
				larger = i;
			}
		}
		previous = i - CL_LEONARNO_NUMBERS[oshift];
		if ( COMPARATOR( vector->data[larger], vector->data[previous] ) >= 0 ) {
			break;
		}
		VALUE_TYPE tmp;
		tmp = vector->data[previous];
		vector->data[previous] = vector->data[i];
		vector->data[i] = tmp;
		do {
			lv >>= 1;
			++shift;
		} while ( ( lv & 1 ) == 0 );
	}

	PREFIX_lheap_rebalance( vector, oshift, i );
}

/**
 * Sorts a vector using the specified comparator.
 *
 * @param vector Pointer to a vector.
 */
static inline void PREFIX_sort( VECTOR * vector ) {
	assert( vector != NULL );
	assert( vector->count < CL_LEONARDO_MAX );

	/* Trivial case */
	if ( vector->count <= 1 ) {
		return;
	}

	/* Heap up */
	CLLeonardoBitvector lv = 1;
	int shift = 1;

	for ( size_t i = 1; i != vector->count; ++i ) {
		assert( ( lv & 1 ) == 1 );
		/* Add next element */
		if ( ( lv & 3 ) == 3 ) {
			lv >>= 2;
			lv |= 1;
			shift += 2;
		} else if ( shift == 1 ) {
			shift = 0;
			lv = ( lv << 1 ) | 1;
		} else {
			lv <<= shift - 1;
			lv |= 1;
			shift = 1;
		}

		/* Fix heap(s) */
		int need_full_fix = 0;
		if ( shift == 0 ) {
			if ( i == vector->count - 1 ) {
				need_full_fix = 1;
			}
		} else if ( shift == 1 ) {
			if ( ( i == vector->count - 1 ) || ( ( i == vector->count - 2 ) && ( ( lv & 2 ) == 0 ) ) ) {
				need_full_fix = 1;
			}
		} else {
			if ( vector->count - i - 1 <= CL_LEONARNO_NUMBERS[shift - 1] ) {
				need_full_fix = 1;
			}
		}
		if ( need_full_fix ) {
			PREFIX_lheap_rectify( vector, lv, shift, i );
		} else {
			PREFIX_lheap_rebalance( vector, shift, i );
		}
	}

	/* Heap down */
	for ( size_t i = vector->count; i != 0; --i ) {
		if ( shift <= 1 ) {
			do {
				lv >>= 1;
				++shift;
			} while ( lv && ( ( lv & 1 ) == 0 ) );
		} else {
			assert( i >= 2 );
			assert( shift >= 2 );
			assert( CL_LEONARNO_NUMBERS[shift - 2] <= i - 2 );
			lv &= ~( ( CLLeonardoBitvector ) 1 );
			lv = ( lv << 2 ) | 3;
			shift -= 2;
			size_t right = i - 2;
			size_t left = right - CL_LEONARNO_NUMBERS[shift];
			PREFIX_lheap_rectify( vector, lv >> 1, shift + 1, left );
			PREFIX_lheap_rectify( vector, lv, shift, right );
		}
	}
}

/**
 * Convenience function to sort a raw array.
 *
 * @param size Size of array.
 * @param array Pointer to an array of the specified size.
 */
static inline void PREFIX_sort_raw( size_t size, VALUE_TYPE * array ) {
	assert( ( size == 0 ) || ( array != NULL ) );

	/* Build fake vector */
	VECTOR fake = ( VECTOR ) {
		.count = size,
		.size = size,
		.data = array,
	};

	PREFIX_sort( &fake );
}

/**
 * Finds an element in a sorted vector.
 *
 * @param vector Pointer to a sorted vector.
 * @param e Element to find.
 *
 * @return The index of the smallest element greater or equal to the one
 * 	specified is returned. If there is no such element, the total number
 * 	of elements in the vector is returned. If there are multiple such
 * 	elements, one of their indices is returned. Which one is unspecified.
 *
 * @sa #PREFIX_sort()
 */
static inline size_t PREFIX_search( const VECTOR * vector, const VALUE_TYPE e ) {
	assert( vector != NULL );

	size_t left = 0;
	size_t right = vector->count;

	while ( left < right ) {
		size_t middle = ( left + right ) / 2;
		assert( middle < vector->count );
		if ( COMPARATOR( vector->data[middle], e ) < 0 ) {
			left = middle + 1;
		} else {
			right = middle;
		}
	}

	return left;
}

/**
 * Convenience function to find an element in a sorted raw array.
 *
 * @param size Size of array.
 * @param array Pointer to a sorted array of the specified size.
 * @param e Element to find.
 *
 * @return The index of the smallest element greater or equal to the one
 * 	specified is returned. If there is no such element, <code>size</code>
 * 	is returned. If there are multiple such elements, one of their indices
 * 	is returned. Which one is unspecified.
 *
 * @sa #PREFIX_sort_raw()
 */
static inline size_t PREFIX_search_raw( size_t size, const VALUE_TYPE * array, const VALUE_TYPE e ) {
	VECTOR fake = ( VECTOR ) {
		.count = size,
		.size = size,
		.data = ( VALUE_TYPE * ) array // contents will not be altered
	};

	return PREFIX_search( &fake, e );
}

#endif // sort implementation

#endif

