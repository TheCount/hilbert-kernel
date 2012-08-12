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

#ifndef HILBERT_PARAM_H__
#define HILBERT_PARAM_H__

#include"private.h"

#include<assert.h>
#include<stdlib.h>

#include"cl/pmap.h"

/**
 * Creates a parameter with empty handle map.
 *
 * @param src Pointer to source module.
 *
 * @return On success, a pointer to the newly created parameter is returned.
 * 	On error, <code>NULL</code> is returned.
 */
static inline union Object * param_create(struct HilbertModule * src) {
	union Object * result = malloc(sizeof(*result));
	if (result == NULL)
		goto noparammem;

	result->param.type = HILBERT_TYPE_PARAM;
	result->param.module = src;
	int rc = hilbert_pmap_init( &result->param.handle_map );
	if ( rc != 0 ) {
		goto nomapmem;
	}

	return result;

nomapmem:
	free(result);
noparammem:
	return NULL;
}

/**
 * Sets a dependency between two modules,
 * such that the destination module depends on the source module,
 * and the source module reverse-depends on the destination module.
 *
 * @param dest Pointer to the destination module, assumed to be locked.
 * @param src Pointer to the source module, assumed to be locked.
 *
 * @return On success, <code>0</code> is returned.
 * 	On error, a negative value is returned, which may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_NOMEM</code>:
 * 			There was not enough memory to perform the operation.
 */
static inline int set_dependency(struct HilbertModule * restrict dest, struct HilbertModule * restrict src) {
	assert (src != NULL);
	assert (dest != NULL);

	int contains_dep  = hilbert_mset_contains( &dest->dependencies, src );
	int contains_rdep = hilbert_mset_contains( &src->reverse_dependencies, dest );

	if ( ( !contains_dep ) && ( hilbert_mset_add( &dest->dependencies, src ) != 0 ) ) {
		return HILBERT_ERR_NOMEM;
	}

	if ( ( !contains_rdep ) && ( hilbert_mset_add( &src->reverse_dependencies, dest ) != 0 ) ) {
		if ( !contains_dep ) {
			hilbert_mset_remove( &dest->dependencies, src );
		}
		return HILBERT_ERR_NOMEM;
	}

	return 0;
}

#endif
