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

#include"private.h"

#include<assert.h>
#include<stdlib.h>

#include"cl/iset.h"
#include"cl/evector.h"
#include"cl/ivector.h"

#include"threads/hthreads.h"

/**
 * Locks all expressions in an array.
 *
 * @param count Number of expressions to be locked.
 * @param expressions Pointer to an array of length <code>count</code> of pointers to expressions to be locked.
 *
 * @return An error code is returned.
 */
static int lock_all_expressions(size_t count, struct HilbertExpression ** expressions) {
	assert ((count == 0) || (expressions != NULL));

	int errcode = 0;
	size_t current;

	for (current = 0; current != count; ++current) {
		if (mtx_lock(&expressions[current]->mutex) != thrd_success) {
			errcode = HILBERT_ERR_INTERNAL;
			break;
		}
	}

	if (errcode != 0) { /* try to undo as much damage as possible */
		while (current--)
			mtx_unlock(&expressions[current]->mutex); /* Ignore further errors */
	}

	return errcode;
}

/**
 * Unlocks all expressions in an array.
 *
 * @param count Number of expressions to be unlocked.
 * @param expressions Pointer to an array of length <code>count</code> of pointers to expressions to be unlocked.
 *
 * @return An error code is returned.
 */
static int unlock_all_expressions(size_t count, struct HilbertExpression ** expressions) {
	assert ((count == 0) || (expressions != NULL));

	int errcode = 0;

	while (count--) {
		if (mtx_unlock(&expressions[count]->mutex) != thrd_success)
			errcode = HILBERT_ERR_INTERNAL;
	}

	return errcode;
}

/**
 * Builds an expression where the head symbol is a functor.
 *
 * @param expr Pointer to an initialised Hilbert expression.
 * @param head Head functor handle.
 * @param count Number of subexpressions.
 * @param subexpr Pointer to an array of pointers to subexpressions.
 *
 * @return An error code is returned.
 */
static int build_expression_from_functor(struct HilbertExpression * restrict expr, HilbertHandle head, size_t count, struct HilbertExpression ** restrict subexpr) {
	assert (expr != NULL);
	assert (expr->module != NULL);
	assert (expr->kindstack == NULL);
	assert ( hilbert_ivector_count( &expr->handles ) == 0 );
	assert ((count == 0) || (subexpr != NULL));

	int errcode;

	if ( hilbert_ivector_pushback( &expr->handles, head ) != 0 ) {
		errcode = HILBERT_ERR_NOMEM;
		goto noheadmem;
	}

	size_t place_count;
	HilbertHandle * ikinds = hilbert_functor_getinputkinds(expr->module, head, &place_count, &errcode);
	if (errcode != 0)
		goto noikinds;
	if (place_count != count) {
		errcode = HILBERT_ERR_COUNT_MISMATCH;
		goto wrongplacecount;
	}

	errcode = lock_all_expressions(count, subexpr);
	if (errcode != 0)
		goto nolock;

	for (size_t i = 0; i != count; ++i) {
		if (subexpr[i]->module != expr->module) {
			errcode = HILBERT_ERR_INVALID_MODULE;
			goto invalidmodule;
		}
		HilbertHandle ekind = hilbert_expression_getkind(subexpr[i], &errcode);
		if (errcode != 0) {
			assert (errcode == HILBERT_ERR_INVALID_EXPR);
			goto unfinishedexpr;
		}
		int rc = hilbert_kind_isequivalent(expr->module, ekind, ikinds[i], &errcode);
		assert (errcode == 0);
		if (!rc) {
			errcode = HILBERT_ERR_NO_EQUIVALENCE;
			goto noequiv;
		}
		if ( hilbert_ivector_append( &expr->handles, &subexpr[i]->handles ) != 0 ) {
			errcode = HILBERT_ERR_NOMEM;
			goto nohandlesmem;
		}
	}

	errcode = 0;
	goto success;

nohandlesmem:
noequiv:
unfinishedexpr:
invalidmodule:
success:
	if (unlock_all_expressions(count, subexpr) != 0)
		errcode = HILBERT_ERR_INTERNAL;
nolock:
wrongplacecount:
	hilbert_harray_free(ikinds);
noikinds:
noheadmem:
	return errcode;
}

struct HilbertExpression * hilbert_expression_create(struct HilbertModule * restrict module, HilbertHandle head, size_t count, struct HilbertExpression ** restrict subexpr, int * restrict errcode) {
	assert (module != NULL);
	assert ((count == 0) || (subexpr != NULL));
	assert (errcode != NULL);

	struct HilbertExpression * expr;

	expr = malloc(sizeof(*expr));
	if (expr == NULL) {
		*errcode = HILBERT_ERR_NOMEM;
		goto noexprmem;
	}

	if (mtx_init(&expr->mutex, mtx_plain | mtx_recursive) != thrd_success) {
		*errcode = HILBERT_ERR_NOMEM;
		goto nomutexmem;
	}

	expr->module = module;
	expr->kindstack = NULL;

	*errcode = hilbert_ivector_init( &expr->handles );
	if ( *errcode != 0 ) {
		*errcode = HILBERT_ERR_NOMEM;
		goto nohandlesmem;
	}

	unsigned int headtype = hilbert_object_gettype(module, head, errcode);
	if (*errcode != 0) {
		*errcode = HILBERT_ERR_INVALID_HANDLE;
		goto headtypeerr;
	}
	assert (((headtype & HILBERT_TYPE_VAR) == 0) || ((headtype & HILBERT_TYPE_FUNCTOR) == 0));
	if (headtype & HILBERT_TYPE_VAR) {
		if (count != 0) {
			*errcode = HILBERT_ERR_COUNT_MISMATCH;
			goto countmismatch;
		}
		if ( hilbert_ivector_pushback( &expr->handles, head ) != 0 ) {
			*errcode = HILBERT_ERR_NOMEM;
			goto noheadmem;
		}
	} else if (headtype & HILBERT_TYPE_FUNCTOR) {
		*errcode = build_expression_from_functor(expr, head, count, subexpr);
		if (*errcode != 0)
			goto functorerr;
	} else {
		*errcode = HILBERT_ERR_INVALID_HANDLE;
		goto headtypeerr;
	}

	*errcode = 0;
	goto success;

functorerr:
noheadmem:
countmismatch:
headtypeerr:
	hilbert_ivector_fini( &expr->handles );
nohandlesmem:
	mtx_destroy(&expr->mutex);
nomutexmem:
	free(expr);
noexprmem:
success:
	return expr;
}

struct HilbertExpression * hilbert_expression_start(struct HilbertModule * restrict module, int * restrict errcode) {
	assert (module != NULL);
	assert (errcode != NULL);

	struct HilbertExpression * expr;

	expr = malloc(sizeof(*expr));
	if (expr == NULL) {
		*errcode = HILBERT_ERR_NOMEM;
		goto noexprmem;
	}

	if (mtx_init(&expr->mutex, mtx_plain | mtx_recursive) != thrd_success) {
		*errcode = HILBERT_ERR_NOMEM;
		goto nomtxmem;
	}

	expr->module = module;

	expr->kindstack = hilbert_ivector_new();
	if (expr->kindstack == NULL) {
		*errcode = HILBERT_ERR_NOMEM;
		goto nokindstackmem;
	}

	*errcode = hilbert_ivector_init( &expr->handles );
	if ( *errcode != 0 ) {
		*errcode = HILBERT_ERR_NOMEM;
		goto nohandlesmem;
	}

	*errcode = 0;
	goto success;

nohandlesmem:
	hilbert_ivector_del(expr->kindstack);
nokindstackmem:
	mtx_destroy(&expr->mutex);
nomtxmem:
	free(expr);
noexprmem:
success:
	return expr;
}

/**
 * Adds a variable to an unfinished Hilbert expression.
 *
 * @param expr Pointer to a locked, unfinished Hilbert expression.
 * @param varhandle Variable handle.
 *
 * @return An error code is returned.
 */
static int add_variable_to_expression(struct HilbertExpression * expr, HilbertHandle varhandle) {
	assert (expr != NULL);
	assert (expr->kindstack != NULL);

	int errcode;
	int rc;

	size_t kssize = hilbert_ivector_count(expr->kindstack);
	if (kssize > 0) {
		HilbertHandle varkind = hilbert_var_getkind(expr->module, varhandle, &errcode);
		assert (errcode == 0);
		HilbertHandle expectedkind = hilbert_ivector_last(expr->kindstack);
		rc = hilbert_kind_isequivalent(expr->module, varkind, expectedkind, &errcode);
		assert (errcode == 0);
		if (!rc) {
			errcode = HILBERT_ERR_NO_EQUIVALENCE;
			goto noeq;
		}
		--kssize; /* expr->kindstack downsized below */
	}

	if ( hilbert_ivector_pushback( &expr->handles, varhandle ) != 0 ) {
		errcode = HILBERT_ERR_NOMEM;
		goto nomem;
	}

	rc = hilbert_ivector_downsize(expr->kindstack, kssize);
	assert (rc == 0);

nomem:
noeq:
	return errcode;
}

/**
 * Adds a functor to an unfinished Hilbert expression.
 *
 * @param expr Pointer to a locked, unfinished Hilbert expression.
 * @param functorhandle Functor handle.
 *
 * @return An error code is returned.
 */
static int add_functor_to_expression(struct HilbertExpression * expr, HilbertHandle functorhandle) {
	assert (expr != NULL);
	assert (expr->kindstack != NULL);

	int errcode;
	int rc;
	HilbertHandle expectedkind;

	size_t ikcount;
	HilbertHandle * ikinds = hilbert_functor_getinputkinds(expr->module, functorhandle, &ikcount, &errcode);
	if (errcode != 0)
		goto ikerror;


	size_t kssize = hilbert_ivector_count(expr->kindstack);
	if (kssize > 0) {
		HilbertHandle functorkind = hilbert_functor_getkind(expr->module, functorhandle, &errcode);
		assert (errcode == 0);
		expectedkind = hilbert_ivector_last(expr->kindstack);
		rc = hilbert_kind_isequivalent(expr->module, functorkind, expectedkind, &errcode);
		assert (errcode == 0);
		if (!rc) {
			errcode = HILBERT_ERR_NO_EQUIVALENCE;
			goto noeq;
		}
		rc = hilbert_ivector_downsize(expr->kindstack, kssize - 1);
		assert (rc == 0);
	}

	size_t oldhsize = hilbert_ivector_count( &expr->handles );
	if ( hilbert_ivector_pushback( &expr->handles, functorhandle ) != 0 ) {
		errcode = HILBERT_ERR_NOMEM;
		goto nohmem;
	}

	size_t newkssize = hilbert_ivector_count(expr->kindstack);
	while (ikcount--) { /* push input kinds on stack in reverse order */
		if (hilbert_ivector_pushback(expr->kindstack, ikinds[ikcount]) != 0) {
			errcode = HILBERT_ERR_NOMEM;
			rc = hilbert_ivector_downsize(expr->kindstack, newkssize);
			assert (rc == 0);
			goto noksmem;
		}
	}

	goto success;

noksmem:
	rc = hilbert_ivector_downsize( &expr->handles, oldhsize );
	assert (rc == 0);
nohmem:
	if (kssize > 0) {
		rc = hilbert_ivector_pushback(expr->kindstack, expectedkind);
		assert (rc == 0);
	}
noeq:
success:
	hilbert_harray_free(ikinds);
ikerror:
	return errcode;
}

/**
 * Adds a handle to an unfinished Hilbert expression.
 *
 * @param expr Pointer to a locked, unfinished Hilbert expression.
 * @param handle Hilbert handle.
 *
 * @return An error code is returned.
 */
static int add_handle_to_expression(struct HilbertExpression * expr, HilbertHandle handle) {
	assert (expr != NULL);
	assert (expr->kindstack != NULL);

	int errcode;

	unsigned int type = hilbert_object_gettype(expr->module, handle, &errcode);
	if (errcode != 0)
		goto typeerr;
	assert (((type & HILBERT_TYPE_VAR) == 0) || ((type & HILBERT_TYPE_FUNCTOR) == 0));

	if (type & HILBERT_TYPE_VAR) {
		errcode = add_variable_to_expression(expr, handle);
	} else if (type & HILBERT_TYPE_FUNCTOR) {
		errcode = add_functor_to_expression(expr, handle);
	} else {
		errcode = HILBERT_ERR_INVALID_HANDLE;
	}

typeerr:
	return errcode;
}

enum HilbertExpressionType hilbert_expression_add(struct HilbertExpression * restrict expr, HilbertHandle handle, int * restrict errcode) {
	assert (expr != NULL);
	assert (errcode != NULL);

	enum HilbertExpressionType result = HILBERT_UNFINISHED_EXPRESSION;

	if (mtx_lock(&expr->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto nolock;
	}

	if (expr->kindstack == NULL) {
		*errcode = HILBERT_ERR_INVALID_EXPR;
		goto invalidexpr;
	}

	*errcode = add_handle_to_expression(expr, handle);
	if ((*errcode == 0) && (hilbert_ivector_count(expr->kindstack) == 0)) {
		hilbert_ivector_del(expr->kindstack);
		expr->kindstack = NULL;
		result = HILBERT_FINISHED_EXPRESSION;
	}

invalidexpr:
	if (mtx_unlock(&expr->mutex) != thrd_success)
		*errcode = HILBERT_ERR_INTERNAL;
nolock:
	return result;
}

struct HilbertExpression * hilbert_expression_fromarray(struct HilbertModule * restrict module, size_t length, const HilbertHandle * restrict handles, int * restrict errcode) {
	assert (module != NULL);
	assert ((length == 0) || (handles != NULL));
	assert (errcode != NULL);

	struct HilbertExpression * expr;

	expr = malloc(sizeof(*expr));
	if (expr == NULL) {
		*errcode = HILBERT_ERR_NOMEM;
		goto noexprmem;
	}

	if (mtx_init(&expr->mutex, mtx_plain | mtx_recursive) != thrd_success) {
		*errcode = HILBERT_ERR_NOMEM;
		goto nomtxmem;
	}

	expr->module = module;

	expr->kindstack = hilbert_ivector_new();
	if (expr->kindstack == NULL) {
		*errcode = HILBERT_ERR_NOMEM;
		goto noksmem;
	}

	*errcode = hilbert_ivector_init( &expr->handles );
	if ( *errcode != 0 ) {
		*errcode = HILBERT_ERR_NOMEM;
		goto nohmem;
	}

	for (size_t i = 0; i != length; ++i) {
		*errcode = add_handle_to_expression(expr, handles[i]);
		if (*errcode != 0)
			goto adderror;
		if (hilbert_ivector_count(expr->kindstack) == 0) {
			if (i + 1 != length) {
				*errcode = HILBERT_ERR_INVALID_EXPR;
				goto invalidexpr;
			}
			hilbert_ivector_del(expr->kindstack);
			expr->kindstack = NULL;
			goto success;
		}
	}

	*errcode = 0;
	goto success;

invalidexpr:
adderror:
	hilbert_ivector_fini( &expr->handles );
nohmem:
	hilbert_ivector_del(expr->kindstack);
noksmem:
	mtx_destroy(&expr->mutex);
nomtxmem:
	free(expr);
noexprmem:
success:
	return expr;
}

enum HilbertExpressionType hilbert_expression_gettype(struct HilbertExpression * restrict expr, int * restrict errcode) {
	assert (expr != NULL);
	assert (errcode != NULL);

	enum HilbertExpressionType type = HILBERT_FINISHED_EXPRESSION;

	if (mtx_lock(&expr->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto nolock;
	}

	if (expr->kindstack != NULL)
		type = HILBERT_UNFINISHED_EXPRESSION;
	*errcode = 0;

	if (mtx_unlock(&expr->mutex) != thrd_success)
		*errcode = HILBERT_ERR_INTERNAL;
nolock:
	return type;
}

struct HilbertModule * hilbert_expression_getmodule(struct HilbertExpression * restrict expr) {
	assert (expr != NULL);

	return expr->module; /* No lock necessary as module remains constant */
}

HilbertHandle hilbert_expression_getkind(struct HilbertExpression * restrict expr, int * restrict errcode) {
	assert (expr != NULL);
	assert (errcode != NULL);

	HilbertHandle kind = 0;

	if (mtx_lock(&expr->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto nolock;
	}

	if (expr->kindstack != NULL) {
		*errcode = HILBERT_ERR_INVALID_EXPR;
		goto unfinishedexpr;
	}

	assert( hilbert_ivector_count( &expr->handles ) > 0 );
	HilbertHandle head = hilbert_ivector_get( &expr->handles, 0 );
	unsigned int type = hilbert_object_gettype(expr->module, head, errcode);
	assert (*errcode == 0);
	assert (type & (HILBERT_TYPE_VAR | HILBERT_TYPE_FUNCTOR));

	if (type & HILBERT_TYPE_VAR) {
		kind = hilbert_var_getkind(expr->module, head, errcode);
	} else {
		kind = hilbert_functor_getkind(expr->module, head, errcode);
	}
	assert (*errcode == 0);

unfinishedexpr:
	if (mtx_unlock(&expr->mutex) != thrd_success)
		*errcode = HILBERT_ERR_INTERNAL;
nolock:
	return kind;
}

struct HilbertExpression ** hilbert_expression_subexpressions(struct HilbertExpression * restrict expr, size_t * restrict count, int * restrict errcode) {
	assert (expr != NULL);
	assert (count != NULL);
	assert (errcode != NULL);

	struct HilbertExpression ** result = NULL;

	ExpressionVector evector;
	*errcode = hilbert_evector_init( &evector );
	if ( *errcode != 0 ) {
		*errcode = HILBERT_ERR_NOMEM;
		goto noevectormem;
	}

	if (mtx_lock(&expr->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto nolock;
	}

	if (expr->kindstack != NULL) {
		*errcode = HILBERT_ERR_INVALID_EXPR;
		goto unfinishedexpr;
	}

	void * i = hilbert_ivector_iterator_start( &expr->handles );
	assert ( i != NULL );
	i = hilbert_ivector_iterator_next( &expr->handles, i ); /* discard head */
	while ( i != NULL ) {
		struct HilbertExpression * subexpr = hilbert_expression_start(expr->module, errcode);
		if (*errcode != 0) {
			assert (*errcode == HILBERT_ERR_NOMEM);
			goto nosubexprmem;
		}
		while ((hilbert_expression_gettype(subexpr, errcode) == HILBERT_UNFINISHED_EXPRESSION) && (*errcode == 0)) {
			assert ( i != NULL );
			hilbert_expression_add( subexpr, hilbert_ivector_iterator_get( &expr->handles, i ), errcode );
			if (*errcode != 0) {
				assert (*errcode == HILBERT_ERR_NOMEM);
				hilbert_expression_free(subexpr);
				goto nosubexprmem;
			}
		}
		if (*errcode != 0) {
			assert (*errcode == HILBERT_ERR_INTERNAL);
			goto subexprinternalerror;
		}
		if ( hilbert_evector_pushback( &evector, subexpr ) != 0 ) {
			*errcode = HILBERT_ERR_NOMEM;
			hilbert_expression_free(subexpr);
			goto nosubexprmem;
		}
	}

	*count = hilbert_evector_count( &evector );
	result = hilbert_evector_toarray( &evector );
	if ((*count > 0) && (result == NULL)) {
		*errcode = HILBERT_ERR_NOMEM;
		goto noarraymem;
	}

	*errcode = 0;
	goto success;

noarraymem:
subexprinternalerror:
nosubexprmem:
	for ( void * i = hilbert_evector_iterator_start( &evector ); i != NULL; i = hilbert_evector_iterator_next( &evector, i) ) {
		hilbert_expression_free( hilbert_evector_iterator_get( &evector, i ) );
	}
	hilbert_evector_downsize( &evector, 0 ); /* avoid double free */
unfinishedexpr:
success:
	if (mtx_unlock(&expr->mutex)) {
		*errcode = HILBERT_ERR_INTERNAL;
		for ( void * i = hilbert_evector_iterator_start( &evector ); i != NULL; i = hilbert_evector_iterator_next( &evector, i ) ) {
			hilbert_expression_free( hilbert_evector_iterator_get( &evector, i ) );
		}
		free(result);
	}
nolock:
	hilbert_evector_fini( &evector );
noevectormem:
	return result;
}

size_t hilbert_expression_getlength(struct HilbertExpression * restrict expr, int * restrict errcode) {
	assert (expr != NULL);
	assert (errcode != NULL);

	size_t result = 0;

	if (mtx_lock(&expr->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto nolock;
	}

	result = hilbert_ivector_count( &expr->handles );
	*errcode = 0;

	if (mtx_unlock(&expr->mutex) != thrd_success)
		*errcode = HILBERT_ERR_INTERNAL;
nolock:
	return result;
}

HilbertHandle * hilbert_expression_toarray(struct HilbertExpression * restrict expr, size_t * restrict length, int * restrict errcode) {
	assert (expr != NULL);
	assert (length != NULL);
	assert (errcode != NULL);

	HilbertHandle * result = NULL;

	if (mtx_lock(&expr->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto nolock;
	}

	*length = hilbert_ivector_count( &expr->handles );
	result = hilbert_ivector_toarray( &expr->handles );
	if ((*length > 0) && (result == NULL)) {
		*errcode = HILBERT_ERR_NOMEM;
		goto nomem;
	}

	*errcode = 0;

nomem:
	if (mtx_unlock(&expr->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		free(result);
	}
nolock:
	return result;
}

HilbertHandle * hilbert_expression_variables(struct HilbertExpression * restrict expr, size_t * restrict count, int * restrict errcode) {
	assert (expr != NULL);
	assert (count != NULL);
	assert (errcode != NULL);

	HilbertHandle * result = NULL;

	IndexSet varset;
	*errcode = hilbert_iset_init( &varset );
	if ( *errcode != 0 ) {
		*errcode = HILBERT_ERR_NOMEM;
		goto novsmem;
	}

	IndexVector varvector;
	*errcode = hilbert_ivector_init( &varvector );
	if ( *errcode != 0 ) {
		*errcode = HILBERT_ERR_NOMEM;
		goto novvmem;
	}

	if (mtx_lock(&expr->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto nolock;
	}

	for ( void * i = hilbert_ivector_iterator_start( &expr->handles ); i != NULL; i = hilbert_ivector_iterator_next( &expr->handles, i ) ) {
		HilbertHandle handle = hilbert_ivector_iterator_get( &expr->handles, i );
		unsigned int type = hilbert_object_gettype(expr->module, handle, errcode);
		if (*errcode != 0) {
			assert (*errcode == HILBERT_ERR_INTERNAL);
			goto typeerr;
		}
		if (type & HILBERT_TYPE_FUNCTOR)
			continue;
		assert (type & HILBERT_TYPE_VAR);
		if ( hilbert_iset_contains( &varset, handle ) ) {
			continue;
		}
		if ( hilbert_iset_add( &varset, handle ) != 0 ) {
			*errcode = HILBERT_ERR_NOMEM;
			goto nohsmem;
		}
		if ( hilbert_ivector_pushback( &varvector, handle ) != 0 ) {
			*errcode = HILBERT_ERR_NOMEM;
			goto nohvmem;
		}
	}

	*count = hilbert_ivector_count( &varvector );
	result = hilbert_ivector_toarray( &varvector );
	if ((*count > 0) && (result == NULL)) {
		*errcode = HILBERT_ERR_NOMEM;
		goto noresultmem;
	}
	*errcode = 0;

noresultmem:
nohvmem:
nohsmem:
typeerr:
	if (mtx_unlock(&expr->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		free(result);
	}
nolock:
	hilbert_ivector_fini( &varvector );
novvmem:
	hilbert_iset_fini( &varset );
novsmem:
	return result;
}

void hilbert_expression_free(struct HilbertExpression * expr) {
	assert (expr != NULL);

	hilbert_ivector_fini( &expr->handles );
	if (expr->kindstack != NULL)
		hilbert_ivector_del(expr->kindstack);
	mtx_destroy(&expr->mutex);
	free(expr);
}
