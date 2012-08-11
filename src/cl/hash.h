/*
 *  The Hilbert Kernel Library, a library for verifying formal proofs.
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

#ifndef CL_HASH_H__
#define CL_HASH_H__

#include<limits.h>
#include<stdint.h>

/**
 * 32-bit hash.
 *
 * @author Robert Jenkins, public domain code
 *
 * @param a 32-bit value to be hashed.
 *
 * @return A hash value for <code>a</code> is returned.
 */
static inline size_t cl_hash32(register uint_fast32_t a) {
	a = (a + 0x7ed55d16UL) + (a << 12);
	a = (a ^ 0xc761c23cUL) ^ (a >> 19);
	a = (a + 0x165667b1UL) + (a <<  5);
	a = (a + 0xd3a2646cUL) ^ (a <<  9);
	a = (a + 0xfd7046c5UL) + (a <<  3);
	a = (a ^ 0xb55a4f09UL) ^ (a >> 16);
	return (size_t) a;
}

// FIXME: are there good free 64-bit hash functions?

/**
 * Mix macro for Jenkins hashing
 *
 * @author Robert Jenkins, public domain code
 *
 * @param a a 32-bit value.
 * @param b a 32-bit value.
 * @param c a 32-bit value.
 *
 * The values of <code>a</code>, <code>b</code> and <code>c</code> will be “mixed”.
 */
#define cl_jenkins_mix(a, b, c) { \
	(a) -= (b); (a) -= (c); (a) ^= ((c) >> 13); \
	(b) -= (c); (b) -= (a); (b) ^= ((a) <<  8); \
	(c) -= (a); (c) -= (b); (c) ^= ((b) >> 13); \
	(a) -= (b); (a) -= (c); (a) ^= ((c) >> 12); \
	(b) -= (c); (b) -= (a); (b) ^= ((a) << 16); \
	(c) -= (a); (c) -= (b); (c) ^= ((b) >>  5); \
	(a) -= (b); (a) -= (c); (a) ^= ((c) >>  3); \
	(b) -= (c); (b) -= (a); (b) ^= ((a) << 10); \
	(c) -= (a); (c) -= (b); (c) ^= ((b) >> 15); \
}

/**
 * Attempt to determine how many bytes there are per hash unit.
 */
#if (CHAR_BIT <= 10)
#define CL_BYTES_PER_U32 4
#elif (CHAR_BIT <= 15)
#define CL_BYTES_PER_U32 3
#elif (CHAR_BIT <= 31)
#define CL_BYTES_PER_U32 2
#else
#define CL_BYTES_PER_U32 1
#endif

/**
 * Hashes arbitrary length keys.
 *
 * @author Robert Jenkins, public domain code
 *
 * @param key pointer to a key.
 * @param size size of key in bytes.
 * @param initval previous hash, or an arbitrary value.
 *
 * @return A hash value for the data pointed to by <code>k</code> is returned.
 */
static inline size_t cl_jenkins_hash(register const void * key, register size_t size, register size_t initval) {
	register const unsigned char * k = key;
	register uint_fast32_t a, b, c;
	register size_t remaining;

	/* Set up the internal state */
	remaining = size;
	a = b = 0x9e3779b9UL; /* the golden ratio; an arbitrary value */
	c = (uint_fast32_t) initval; /* the previous hash value */

	assert (sizeof(uint_fast32_t) >= CL_BYTES_PER_U32);

	/*---------------------------------------- handle most of the key */
	for(remaining = size; remaining >= 3 * CL_BYTES_PER_U32;
			remaining -= 3 * CL_BYTES_PER_U32, k += 3 * CL_BYTES_PER_U32) {
		for (size_t i = 0; i != CL_BYTES_PER_U32; ++i) {
			a += ((uint_fast32_t) k[i]) << (i * CHAR_BIT);
			b += ((uint_fast32_t) k[CL_BYTES_PER_U32 + i]) << (i * CHAR_BIT);
			c += ((uint_fast32_t) k[2 * CL_BYTES_PER_U32 + i]) << (i * CHAR_BIT);
		}
		cl_jenkins_mix(a, b, c);
	}

	/*------------------------ handle the last few (maximum 11) bytes */
	c += size;
	switch (remaining) {      /* all the case statements fall through */
#if (CL_BYTES_PER_U32 == 4)
		case 11: c += ((uint_fast32_t) k[10]) << (3 * CHAR_BIT);
		case 10: c += ((uint_fast32_t) k[ 9]) << (2 * CHAR_BIT);
		case  9: c += ((uint_fast32_t) k[ 8]) << (1 * CHAR_BIT);
		case  8: b += ((uint_fast32_t) k[ 7]) << (3 * CHAR_BIT);
		case  7: b += ((uint_fast32_t) k[ 6]) << (2 * CHAR_BIT);
		case  6: b += ((uint_fast32_t) k[ 5]) << (1 * CHAR_BIT);
		case  5: b += ((uint_fast32_t) k[ 4]) << (0 * CHAR_BIT);
		case  4: a += ((uint_fast32_t) k[ 3]) << (3 * CHAR_BIT);
		case  3: a += ((uint_fast32_t) k[ 2]) << (2 * CHAR_BIT);
		case  2: a += ((uint_fast32_t) k[ 1]) << (1 * CHAR_BIT);
		case  1: a += ((uint_fast32_t) k[ 0]) << (0 * CHAR_BIT);
#elif (CL_BYTES_PER_U32 == 3)
		case  8: c += ((uint_fast32_t) k[ 7]) << (2 * CHAR_BIT);
		case  7: c += ((uint_fast32_t) k[ 6]) << (1 * CHAR_BIT);
		case  6: b += ((uint_fast32_t) k[ 5]) << (2 * CHAR_BIT);
		case  5: b += ((uint_fast32_t) k[ 4]) << (1 * CHAR_BIT);
		case  4: b += ((uint_fast32_t) k[ 3]) << (0 * CHAR_BIT);
		case  3: a += ((uint_fast32_t) k[ 2]) << (2 * CHAR_BIT);
		case  2: a += ((uint_fast32_t) k[ 1]) << (1 * CHAR_BIT);
		case  1: a += ((uint_fast32_t) k[ 0]) << (0 * CHAR_BIT);
#elif (CL_BYTES_PER_U32 == 2)
		case  5: c += ((uint_fast32_t) k[ 4]) << (1 * CHAR_BIT);
		case  4: b += ((uint_fast32_t) k[ 3]) << (1 * CHAR_BIT);
		case  3: b += ((uint_fast32_t) k[ 2]) << (0 * CHAR_BIT);
		case  2: a += ((uint_fast32_t) k[ 1]) << (1 * CHAR_BIT);
		case  1: a += ((uint_fast32_t) k[ 0]) << (0 * CHAR_BIT);
#elif (CL_BYTES_PER_U32 == 1)
		case  2: b += ((uint_fast32_t) k[ 1]) << (0 * CHAR_BIT);
		case  1: a += ((uint_fast32_t) k[ 0]) << (0 * CHAR_BIT);
#else
#error "Oops, something is wrong in the Jenkins hash function"
#endif
		/* case 0: nothing left to add */
	}
	cl_jenkins_mix(a, b, c);

	/*-------------------------------------------- report the result */
	return c;
}

/**
 * Hashes a <code>size_t</code>.
 *
 * @param value a <code>size_t</code> value to be hashed.
 *
 * @return A hash value for <code>value</code> is returned.
 */
static inline size_t cl_hash_index(register size_t value) {
	return cl_hash32((uint_fast32_t) value);
}

/**
 * Hashes a pointer.
 *
 * @param p pointer to be hashed.
 *
 * @return A hash value for <code>p</code> is returned.
 */
static inline size_t cl_hash_pointer(register const void * p) {
#if (defined(UINTPTR_MAX) && (UINTPTR_MAX <= UINT_FAST32_MAX))
	return cl_hash32((uint_fast32_t) (uintptr_t) p);
#else
	/* do it the hard way */
	return cl_jenkins_hash(&p, sizeof(p), 0);
#endif
}

#endif
