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

#ifndef CL_UTIL_H__
#define CL_UTIL_H__

#include<limits.h>
#include<stddef.h>

/**
 * Unused result warning.
 */
#ifdef __GNUC__
#define WARN_UNUSED_RESULT __attribute__( ( warn_unused_result ) )
#else
#define WARN_UNUSED_RESULT /* nothing */
#endif

/**
 * Rounds the specified number up to the next power of two.
 *
 * @param x Number.
 *
 * @return On success, the smallest power of two greater than <code>x</code>
 * 	is returned.
 * 	On error, 0 is returned.
 */
static inline size_t roundup2( size_t x ) {
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
#if ( SIZE_MAX > 0xFFFF )
	x |= x >> 16;
#endif
#if ( SIZE_MAX > 0xFFFFFFFF )
	x |= x >> 32;
#endif
#if ( SIZE_MAX > 0xFFFFFFFFFFFFFFFF )
	x |= x >> 64;
#endif
	++x;

	assert( ( x > 0 ) && ( ( x & ( x - 1 ) ) == 0 ) );
	return x;
}

#endif
