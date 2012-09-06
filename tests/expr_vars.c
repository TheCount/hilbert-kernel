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

/**
 * Test for expression variables.
 */

#include<assert.h>

#include"hilbert.h"

int main( void ) {
	int errcode;
	HilbertModule * module = hilbert_module_create( HILBERT_INTERFACE_MODULE );
	assert( module != NULL );
	HilbertHandle kind = hilbert_kind_create( module, &errcode );
	assert( errcode == 0 );
	HilbertHandle v1 = hilbert_var_create( module, kind, &errcode );
	assert( errcode == 0 );
	HilbertHandle v2 = hilbert_var_create( module, kind, &errcode );
	assert( errcode == 0 );
	HilbertHandle cf = hilbert_functor_create( module, kind, 0, NULL, &errcode );
	assert( errcode == 0 );
	HilbertHandle hv[2] = { kind, kind };
	HilbertHandle f = hilbert_functor_create( module, kind, 2, hv, &errcode );
	assert( errcode == 0 );

	/* Zero-length expressions */
	HilbertExpression * expr = hilbert_expression_start( module, &errcode );
	assert( errcode == 0 );
	size_t count;
	HilbertHandle * harray = hilbert_expression_variables( expr, &count, &errcode );
	assert( errcode == 0 );
	assert( count == 0 );
	hilbert_harray_free( harray );
	hilbert_expression_free( expr );

	/* Length-one expressions */
	expr = hilbert_expression_create( module, cf, 0, NULL, &errcode );
	assert( errcode == 0 );
	harray = hilbert_expression_variables( expr, &count, &errcode );
	assert( errcode == 0 );
	assert( count == 0 );
	hilbert_harray_free( harray );
	hilbert_expression_free( expr );
	expr = hilbert_expression_create( module, v1, 0, NULL, &errcode );
	assert( errcode == 0 );
	harray = hilbert_expression_variables( expr, &count, &errcode );
	assert( errcode == 0 );
	assert( count == 1 );
	assert( harray != NULL );
	assert( harray[0] == v1 );
	hilbert_harray_free( harray );
	hilbert_expression_free( expr );
	expr = hilbert_expression_start( module, &errcode );
	assert( errcode == 0 );
	hilbert_expression_add( expr, f, &errcode );
	assert( errcode == 0 );
	harray = hilbert_expression_variables( expr, &count, &errcode );
	assert( errcode == 0 );
	assert( count == 0 );
	hilbert_harray_free( harray );
	hilbert_expression_free( expr );

	/* Longer expressions */
	expr = hilbert_expression_start( module, &errcode );
	assert( errcode == 0 );
	hilbert_expression_add( expr, f, &errcode );
	assert( errcode == 0 );
	hilbert_expression_add( expr, v1, &errcode );
	assert( errcode == 0 );
	harray = hilbert_expression_variables( expr, &count, &errcode );
	assert( errcode == 0 );
	assert( count == 1 );
	assert( harray != NULL );
	assert( harray[0] == v1 );
	hilbert_harray_free( harray );
	hilbert_expression_free( expr );
	HilbertExpression * subexpr0 = hilbert_expression_create( module, cf, 0, NULL, &errcode );
	assert( errcode == 0 );
	HilbertExpression * subexpr1 = hilbert_expression_create( module, v1, 0, NULL, &errcode );
	assert( errcode == 0 );
	HilbertExpression * subexpr2 = hilbert_expression_create( module, v2, 0, NULL, &errcode );
	assert( errcode == 0 );
	HilbertExpression * ev[2] = { subexpr0, subexpr2 };
	expr = hilbert_expression_create( module, f, 2, ev, &errcode );
	assert( errcode == 0 );
	harray = hilbert_expression_variables( expr, &count, &errcode );
	assert( errcode == 0 );
	assert( count == 1 );
	assert( harray != NULL );
	assert( harray[0] == v2 );
	hilbert_harray_free( harray );
	hilbert_expression_free( expr );
	ev[0] = subexpr1;
	expr = hilbert_expression_create( module, f, 2, ev, &errcode );
	assert( errcode == 0 );
	harray = hilbert_expression_variables( expr, &count, &errcode );
	assert( errcode == 0 );
	assert( count == 2 );
	assert( harray != NULL );
	assert( ( harray[0] == v1 ) && ( harray[1] == v2 ) );
	hilbert_harray_free( harray );
	hilbert_expression_free( expr );
	ev[1] = subexpr1;
	expr = hilbert_expression_create( module, f, 2, ev, &errcode );
	assert( errcode == 0 );
	harray = hilbert_expression_variables( expr, &count, &errcode );
	assert( errcode == 0 );
	assert( count == 1 );
	assert( harray != NULL );
	assert( harray[0] == v1 );
	hilbert_harray_free( harray );
	hilbert_expression_free( expr );
	hilbert_expression_free( subexpr0 );
	hilbert_expression_free( subexpr1 );
	hilbert_expression_free( subexpr2 );

	/* Cleanup */
	hilbert_module_free( module );
}
