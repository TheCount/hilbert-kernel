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
 * Test to check expression properties.
 */

#include<assert.h>

#include"hilbert.h"

int main( void ) {
	int errcode;
	HilbertModule * module = hilbert_module_create( HILBERT_INTERFACE_MODULE );
	assert( module != NULL );
	HilbertHandle kind1 = hilbert_kind_create( module, &errcode );
	assert( errcode == 0 );
	HilbertHandle kind2 = hilbert_kind_create( module, &errcode );
	assert( errcode == 0 );
	HilbertHandle kind3 = hilbert_kind_create( module, &errcode );
	assert( errcode == 0 );
	HilbertHandle vkind = hilbert_vkind_create( module, &errcode );
	assert( errcode == 0 );
	HilbertHandle v1 = hilbert_var_create( module, kind1, &errcode );
	assert( errcode == 0 );
	HilbertHandle v2 = hilbert_var_create( module, kind2, &errcode );
	assert( errcode == 0 );
	HilbertHandle vv = hilbert_var_create( module, vkind, &errcode );
	assert( errcode == 0 );
	HilbertHandle cf = hilbert_functor_create( module, kind1, 0, NULL, &errcode );
	assert( errcode == 0 );
	HilbertHandle hv[2] = { kind1, kind2 };
	HilbertHandle f = hilbert_functor_create( module, kind3, 2, hv, &errcode );
	assert( errcode == 0 );

	/* Expression type */
	HilbertExpression * expr = hilbert_expression_start( module, &errcode );
	assert( errcode == 0 );
	enum HilbertExpressionType type = hilbert_expression_gettype( expr, &errcode );
	assert( errcode == 0 );
	assert( type == HILBERT_UNFINISHED_EXPRESSION );
	type = hilbert_expression_add( expr, cf, &errcode );
	assert( errcode == 0 );
	assert( type == HILBERT_FINISHED_EXPRESSION );
	hilbert_expression_free( expr );
	hv[0] = f;
	hv[1] = cf;
	expr = hilbert_expression_fromarray( module, 2, hv, &errcode );
	assert( errcode == 0 );
	type = hilbert_expression_gettype( expr, &errcode );
	assert( errcode == 0 );
	assert( type == HILBERT_UNFINISHED_EXPRESSION );
	type = hilbert_expression_add( expr, v2, &errcode );
	assert( errcode == 0 );
	assert( type == HILBERT_FINISHED_EXPRESSION );
	hilbert_expression_free( expr );

	/* Expression module */
	expr = hilbert_expression_start( module, &errcode );
	assert( errcode == 0 );
	assert( hilbert_expression_getmodule( expr ) == module );
	hilbert_expression_free( expr );

	/* Expression kind */
	expr = hilbert_expression_start( module, &errcode );
	assert( errcode == 0 );
	HilbertHandle ekind = hilbert_expression_getkind( expr, &errcode );
	assert( errcode == HILBERT_ERR_INVALID_EXPR );
	hilbert_expression_free( expr );
	expr = hilbert_expression_create( module, vv, 0, NULL, &errcode );
	assert( errcode == 0 );
	ekind = hilbert_expression_getkind( expr, &errcode );
	assert( errcode == 0 );
	assert( ekind == vkind );
	hilbert_expression_free( expr );
	expr = hilbert_expression_fromarray( module, 2, hv, &errcode );
	assert( errcode == 0 );
	ekind = hilbert_expression_getkind( expr, &errcode );
	assert( errcode == HILBERT_ERR_INVALID_EXPR );
	hilbert_expression_add( expr, v2, &errcode );
	assert( errcode == 0 );
	ekind = hilbert_expression_getkind( expr, &errcode );
	assert( errcode == 0 );
	assert( ekind == kind3 );
	hilbert_expression_free( expr );

	/* Expression length */
	expr = hilbert_expression_start( module, &errcode );
	assert( errcode == 0 );
	size_t length = hilbert_expression_getlength( expr, &errcode );
	assert( errcode == 0 );
	assert( length == 0 );
	hilbert_expression_add( expr, vv, &errcode );
	assert( errcode == 0 );
	length = hilbert_expression_getlength( expr, &errcode );
	assert( errcode == 0 );
	assert( length == 1 );
	hilbert_expression_free( expr );
	expr = hilbert_expression_fromarray( module, 2, hv, &errcode );
	assert( errcode == 0 );
	length = hilbert_expression_getlength( expr, &errcode );
	assert( errcode == 0 );
	assert( length == 2 );
	hilbert_expression_add( expr, v2, &errcode );
	assert( errcode == 0 );
	length = hilbert_expression_getlength( expr, &errcode );
	assert( errcode == 0 );
	assert( length == 3 );
	hilbert_expression_free( expr );

	/* Cleanup */
	hilbert_module_free( module );
}
