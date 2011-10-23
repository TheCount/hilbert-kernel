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
 * Replacements from vector container template:
 * - <code>VECTOR</code>:
 *   Vector type name,
 * - <code>VALUE_TYPE</code>:
 *   Type name of the value type,
 * - <code>VITER</code>:
 *   Vector iterator type name.
 * - <code>PREFIX</code>:
 *   Prefix of function names.
 */

#ifndef HILBERT_CL_VECTOR_H__
#define HILBERT_CL_VECTOR_H__

#include<assert.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>

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
 * Vector iterator structure.
 */
struct VITER {
	/**
	 * Pointer to vector over which we are iterating.
	 */
	struct VECTOR * vector;

	/**
	 * Current iteration index.
	 */
	size_t index;
};

typedef struct VITER VITER;

/**
 * Creates a new, empty vector.
 *
 * @return On success, a pointer to a new, empty vector is returned.
 * 	On error, <code>NULL</code> is returned.
 */
static inline VECTOR * PREFIX_new(void) {
	VECTOR * result;

	result = malloc(sizeof(*result));
	if (result == NULL)
		return NULL;
	result->count = 0;
	result->size = 1;
	result->data = malloc(result->size * sizeof(*result->data));
	if (result->data == NULL) {
		free(result);
		return NULL;
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

	free(vector->data);
	free(vector);
}

/**
 * Grows a vector (private).
 *
 * @param vector Pointer to the vector to be grown.
 *
 * @return On success, <code>0</code> is returned.
 * 	On error, <code>-1</code> is returned.
 */
static inline int PREFIX_grow(VECTOR * vector) {
	assert (vector != NULL);

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

	return 0;
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
static inline int PREFIX_pushback(VECTOR * vector, VALUE_TYPE elt) {
	assert (vector != NULL);
	assert (vector->count < SIZE_MAX);

	size_t newcount = vector->count + 1;
	if (newcount > vector->size) {
		if (PREFIX_grow(vector) != 0)
			return -1;
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
 * Appends a copy of a vector to the end of a vector.
 *
 * @param dest Pointer to the vector to which a copy of another vector is to be appended.
 * @param src Pointer to the vector a copy of which is to be appended to <code>dest</code>.
 *
 * @return On success, <code>0</code> is returned.
 * 	On error, <code>-1</code> is returned.
 */
static inline int PREFIX_append(VECTOR * dest, VECTOR * src) {
	assert (dest != NULL);
	assert (src != NULL);

	size_t newcount = dest->count + src->count;
	if (newcount < dest->count)
		return -1;

	while (newcount > dest->size) {
		if (PREFIX_grow(dest) != 0)
			return -1; // FIXME: doesn't return possibly grossly overallocated memory
	}

	memcpy(dest->data + dest->count, src->data, src->count * sizeof(*src->data));
	dest->count = newcount;

	return 0;
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
 * Returns the last element from a vector.
 *
 * @param vector Pointer to the vector from which the last element is to be returned.
 * 	If the vector does not have any elements, the behaviour is undefined.
 *
 * @return The last element of the vector is returned.
 */
static inline VALUE_TYPE PREFIX_last(const VECTOR * vector) {
	assert (vector != 0);
	assert (vector->count > 0);

	return vector->data[vector->count - 1];
}

/**
 * Returns an array containing a copy of the elements of a vector.
 *
 * @param vector Pointer to the vector a copy of whose elements is to be returned as an array.
 *
 * @return On success, a pointer to an array with a copy of the elements from the vector pointed to by <code>vector</code> is returned.
 * 	If the vector does not contain any elemnts, this may be <code>NULL</code>.
 * 	The returned array must be freed by the user.
 * 	On error, <code>NULL</code> is returned.
 */
static inline VALUE_TYPE * PREFIX_toarray(const VECTOR * vector) {
	assert (vector != NULL);

	size_t allocsize = vector->count * sizeof(*vector->data);
	VALUE_TYPE * result = malloc(allocsize);
	if (result == NULL)
		return NULL;

	memcpy(result, vector->data, allocsize);

	return result;
}

/**
 * Returns a new iterator over a vector.
 *
 * @param vector Pointer to the vector for which an iterator is to be returned.
 *
 * @return An iterator over the specified vector is returned.
 */
static inline VITER PREFIX_iterator_new(VECTOR * vector) {
	assert (vector != NULL);

	return (VITER) { .vector = vector, .index = 0 };
}

/**
 * Checks whether an iterator over a vector has a next element.
 *
 * @param i Pointer to an iterator over a vector.
 *
 * @return If there is a next element in the iteration, <code>1</code> is returned.
 * 	Otherwise, <code>0</code> is returned.
 */
static inline int PREFIX_iterator_hasnext(VITER * i) {
	assert (i != NULL);

	return (i->index < i->vector->count);
}

/**
 * Returns the next element of an iteration over a vector.
 *
 * @param i Pointer to an iterator over a vector.
 *
 * @return The next element in the iteration is returned.
 * 	If there is no next element, the behaviour is undefined.
 */
static inline VALUE_TYPE PREFIX_iterator_next(VITER * i) {
	assert (i != NULL);

	return i->vector->data[i->index++];
}

#endif
