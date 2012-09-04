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
 * Test to check expression creation.
 */

#include<assert.h>

#include"hilbert.h"

int main( void ) {
	int errcode;
	HilbertModule * module = hilbert_module_create( HILBERT_INTERFACE_MODULE );
	assert( module != NULL );
	HilbertModule * other_module = hilbert_module_create( HILBERT_INTERFACE_MODULE );
	assert( other_module != NULL );
	HilbertHandle kind1 = hilbert_kind_create( module, &errcode );
	assert( errcode == 0 );
	HilbertHandle kind2 = hilbert_kind_create( module, &errcode );
	assert( errcode == 0 );
	HilbertHandle kind3 = hilbert_kind_create( module, &errcode );
	assert( errcode == 0 );
	HilbertHandle vkind = hilbert_vkind_create( module, &errcode );
	assert( errcode == 0 );
	HilbertHandle other_kind = hilbert_kind_create( other_module, &errcode );
	assert( errcode == 0 );
	HilbertHandle v1 = hilbert_var_create( module, kind1, &errcode );
	assert( errcode == 0 );
	HilbertHandle v2 = hilbert_var_create( module, kind2, &errcode );
	assert( errcode == 0 );
	HilbertHandle vv = hilbert_var_create( module, vkind, &errcode );
	assert( errcode == 0 );
	HilbertHandle other_var = hilbert_var_create( other_module, other_kind, &errcode );
	assert( errcode == 0 );
	HilbertHandle cf = hilbert_functor_create( module, kind1, 0, NULL, &errcode );
	assert( errcode == 0 );
	HilbertHandle hv[2] = { kind1, kind2 };
	HilbertHandle f = hilbert_functor_create( module, kind3, 2, hv, &errcode );
	assert( errcode == 0 );
	HilbertHandle other_cf = hilbert_functor_create( other_module, other_kind, 0, NULL, &errcode );
	assert( errcode == 0 );

	/* Wrong handle */
	HilbertExpression * expr = hilbert_expression_create( module, 666, 0, NULL, &errcode );
	assert( errcode == HILBERT_ERR_INVALID_HANDLE );
	expr = hilbert_expression_create( module, kind1, 0, NULL, &errcode );
	assert( errcode == HILBERT_ERR_INVALID_HANDLE );
	expr = hilbert_expression_create( module, vkind, 0, NULL, &errcode );
	assert( errcode == HILBERT_ERR_INVALID_HANDLE );

	/* Simple expressions */
	expr = hilbert_expression_create( module, v1, 0, NULL, &errcode );
	assert( errcode == 0 );
	hilbert_expression_free( expr );
	expr = hilbert_expression_create( module, v2, 0, NULL, &errcode );
	assert( errcode == 0 );
	hilbert_expression_free( expr );
	expr = hilbert_expression_create( module, cf, 0, NULL, &errcode );
	assert( errcode == 0 );
	hilbert_expression_free( expr );

	/* More complicated epressions */
	HilbertExpression * subexpr1 = hilbert_expression_create( module, v1, 0, NULL, &errcode );
	assert( errcode == 0 );
	HilbertExpression * subexpr2 = hilbert_expression_create( module, v2, 0, NULL, &errcode );
	assert( errcode == 0 );
	HilbertExpression * ev[2] = { subexpr1, subexpr2 };
	expr = hilbert_expression_create( module, v1, 1, ev, &errcode );
	assert( errcode == HILBERT_ERR_COUNT_MISMATCH );
	expr = hilbert_expression_create( module, f, 1, ev, &errcode );
	assert( errcode == HILBERT_ERR_COUNT_MISMATCH );
	expr = hilbert_expression_create( module, f, 2, ev, &errcode );
	assert( errcode == 0 );
	hilbert_expression_free( expr );
	hilbert_expression_free( subexpr1 );
	subexpr1 = hilbert_expression_create( other_module, other_cf, 0, NULL, &errcode );
	assert( errcode == 0 );
	ev[0] = subexpr1;
	expr = hilbert_expression_create( module, f, 2, ev, &errcode );
	assert( errcode == HILBERT_ERR_INVALID_MODULE );
	hilbert_expression_free( subexpr1 );
	subexpr1 = hilbert_expression_create( module, cf, 0, NULL, &errcode );
	assert( errcode == 0 );
	ev[0] = subexpr1;
	expr = hilbert_expression_create( module, f, 2, ev, &errcode );
	assert( errcode == 0 );
	hilbert_expression_free( expr );
	hilbert_expression_free( subexpr1 );
	ev[0] = subexpr2;
	expr = hilbert_expression_create( module, f, 2, ev, &errcode );
	assert( errcode = HILBERT_ERR_NO_EQUIVALENCE );
	errcode = hilbert_kind_identify( module, kind1, kind2 );
	assert( errcode == 0 );
	expr = hilbert_expression_create( module, f, 2, ev, &errcode );
	assert( errcode == 0 );
	hilbert_expression_free( expr );
	subexpr1 = hilbert_expression_start( module, &errcode );
	assert( errcode == 0 );
	ev[0] = subexpr1;
	expr = hilbert_expression_create( module, f, 2, ev, &errcode );
	assert( errcode == HILBERT_ERR_INVALID_EXPR );
	hilbert_expression_add( subexpr1, v1, &errcode );
	assert( errcode == 0 );
	expr = hilbert_expression_create( module, f, 2, ev, &errcode );
	assert( errcode == 0 );
	hilbert_expression_free( expr );
	hilbert_expression_free( subexpr1 );
	hilbert_expression_free( subexpr2 );

	/* Cleanup */
	hilbert_module_free( other_module );
	hilbert_module_free( module );
}
