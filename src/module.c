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
#include"cl/mset.h"
#include"cl/ivector.h"
#include"cl/ovector.h"

#include"threads/hthreads.h"

HilbertModule * hilbert_module_create(enum HilbertModuleType type) {
	struct HilbertModule * module;
	int errcode;

	if ((type != HILBERT_INTERFACE_MODULE) && (type != HILBERT_PROOF_MODULE))
		goto wrongtype;

	module = malloc(sizeof(*module));
	if (module == NULL)
		goto allocfail;

	/* init members */

	module->type = type;

	errcode = mtx_init(&module->mutex, mtx_plain | mtx_recursive);
	if (errcode != thrd_success)
		goto mutexfail;

	module->immutable = 0;
	module->freeable = 0;
	module->ancillary = NULL;

	errcode = hilbert_ovector_init( &module->objects );
	if ( errcode != 0 )
		goto noobjectmem;

	errcode = hilbert_ivector_init( &module->kindhandles );
	if ( errcode != 0 )
		goto nokindhandlesmem;

	errcode = hilbert_ivector_init( &module->varhandles );
	if ( errcode != 0 )
		goto novarhandlesmem;

	errcode = hilbert_ivector_init( &module->functorhandles );
	if ( errcode != 0 )
		goto nofunctorhandlesmem;

	errcode = hilbert_ivector_init( &module->paramhandles );
	if ( errcode != 0 ) {
		goto noparamhandlesmem;
	}

	errcode = hilbert_mset_init( &module->dependencies );
	if ( errcode != 0 ) {
		goto nodepmem;
	}

	errcode = hilbert_mset_init( &module->reverse_dependencies );
	if ( errcode != 0 ) {
		goto noreversedepmem;
	}

	return module;

noreversedepmem:
	hilbert_mset_fini( &module->dependencies );
nodepmem:
	hilbert_ivector_fini( &module->paramhandles );
noparamhandlesmem:
	hilbert_ivector_fini( &module->functorhandles );
nofunctorhandlesmem:
	hilbert_ivector_fini( &module->varhandles );
novarhandlesmem:
	hilbert_ivector_fini( &module->kindhandles );
nokindhandlesmem:
	hilbert_ovector_fini( &module->objects );
noobjectmem:
	mtx_destroy(&module->mutex);
mutexfail:
	free(module);
allocfail:
wrongtype:
	return NULL;
}

void hilbert_module_free(struct HilbertModule * module) {
	assert (module != NULL);
	int rc;

	/* Remove module from dependencies and possibly deallocate them */
	rc = mtx_lock(&module->mutex);
	assert (rc == thrd_success);

	module->freeable = 1;

	for ( void * i = hilbert_mset_iterator_start( &module->dependencies ); i != NULL; i = hilbert_mset_iterator_next( &module->dependencies, i ) ) {
		struct HilbertModule * dependency = hilbert_mset_iterator_get( &module->dependencies, i );
		rc = mtx_lock(&dependency->mutex);
		assert (rc == thrd_success);
		rc = hilbert_mset_remove( &dependency->reverse_dependencies, module );
		assert (rc);
		int freeable = dependency->freeable;
		size_t count = hilbert_mset_count( &dependency->reverse_dependencies );
		rc = mtx_unlock(&dependency->mutex);
		assert (rc == thrd_success);
		hilbert_mset_remove( &module->dependencies, dependency );
		if (freeable && (count == 0)) {
			/* dependency is freeable and its dependencies and reverse dependencies are empty.
			 * Hence it can be freed definitely this time. */
			hilbert_module_free(dependency);
		}
	}

	/* return if we still have reverse dependencies and leave final freeing to them */
	size_t count = hilbert_mset_count( &module->reverse_dependencies );
	rc = mtx_unlock(&module->mutex);
	assert (rc == thrd_success);
	if (count != 0)
		return;

	/* free kind equivalence classes */
	for ( void * i = hilbert_ivector_iterator_start( &module->kindhandles ); i != NULL; i = hilbert_ivector_iterator_next( &module->kindhandles, i ) ) {
		union Object * object = hilbert_ovector_get( &module->objects, hilbert_ivector_iterator_get( &module->kindhandles, i ) );
		assert (object->generic.type & HILBERT_TYPE_KIND);
		IndexSet * equivalence_class = object->kind.equivalence_class;
		if (equivalence_class == NULL)
			continue;
		for ( void * j = hilbert_iset_iterator_start( equivalence_class ); j != NULL; j = hilbert_iset_iterator_next( equivalence_class, j ) ) {
			union Object * object2 = hilbert_ovector_get( &module->objects, hilbert_iset_iterator_get( equivalence_class, j ) );
			assert (object2->generic.type & HILBERT_TYPE_KIND);
			object2->kind.equivalence_class = NULL;
		}
		hilbert_iset_del(equivalence_class);
	}

	/* free objects */
	for ( void * i = hilbert_ovector_iterator_start( &module->objects ); i != NULL; i = hilbert_ovector_iterator_next( &module->objects, i ) ) {
		hilbert_object_free( hilbert_ovector_iterator_get( &module->objects, i ) );
	}

	/* free other stuff */
	hilbert_mset_fini( &module->reverse_dependencies );
	hilbert_mset_fini( &module->dependencies );
	hilbert_ivector_fini( &module->paramhandles );
	hilbert_ivector_fini( &module->functorhandles );
	hilbert_ivector_fini( &module->varhandles );
	hilbert_ivector_fini( &module->kindhandles );
	hilbert_ovector_fini( &module->objects );
	mtx_destroy(&module->mutex);
	free(module);
}

enum HilbertModuleType hilbert_module_gettype(struct HilbertModule * module) {
	assert (module != NULL);

	/* Locking the module is not necessary because type is constant during its lifetime */
	return module->type;
}

int hilbert_module_makeimmutable(struct HilbertModule * module) {
	assert (module != NULL);

	int errcode = 0; // no error

	if (hilbert_module_gettype(module) != HILBERT_INTERFACE_MODULE) {
		errcode = HILBERT_ERR_INVALID_MODULE;
		goto wrongtype;
	}

	if (mtx_lock(&module->mutex) != thrd_success) {
		errcode = HILBERT_ERR_INTERNAL;
		goto lockerror;
	}

	if (module->immutable) {
		errcode = HILBERT_ERR_IMMUTABLE;
	} else {
		module->immutable = 1;
	}

	if (mtx_unlock(&module->mutex) != thrd_success)
		errcode = HILBERT_ERR_INTERNAL;

lockerror:
wrongtype:
	return errcode;
}

int hilbert_module_isimmutable(struct HilbertModule * restrict module, int * restrict errcode) {
	assert (module != NULL);
	assert (errcode != NULL);

	int rc = 0;

	if (mtx_lock(&module->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto lockerror;
	}

	rc = module->immutable;

	if (mtx_unlock(&module->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto lockerror;
	}

	*errcode = 0;
lockerror:
	return rc;
}

int hilbert_module_setancillary(struct HilbertModule * module, void * newdata, void ** olddata) {
	assert (module != NULL);

	int errcode;

	if (mtx_lock(&module->mutex) != thrd_success) {
		errcode = HILBERT_ERR_INTERNAL;
		goto lockerror;
	}

	if (olddata != NULL)
		*olddata = module->ancillary;
	module->ancillary = newdata;

	if (mtx_unlock(&module->mutex) != thrd_success) {
		errcode = HILBERT_ERR_INTERNAL;
		goto lockerror;
	}

	errcode = 0;
lockerror:
	return errcode;
}

int hilbert_module_getancillary(struct HilbertModule * module, void ** data) {
	assert (module != NULL);
	assert (data != NULL);

	int errcode = 0; // no error

	if (mtx_lock(&module->mutex) != thrd_success) {
		errcode = HILBERT_ERR_INTERNAL;
		goto lockerror;
	}

	*data = module->ancillary;

	if (mtx_unlock(&module->mutex) != thrd_success)
		errcode = HILBERT_ERR_INTERNAL;

lockerror:
	return errcode;
}

