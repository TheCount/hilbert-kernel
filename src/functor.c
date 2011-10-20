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

#include"private.h"

#include<assert.h>
#include<stdlib.h>
#include<string.h>

#include"cl/ivector.h"
#include"cl/ovector.h"

#include"threads/hthreads.h"

HilbertHandle hilbert_functor_create(struct HilbertModule * restrict module, HilbertHandle rkindhandle, size_t count, const HilbertHandle * restrict ikindhandles, int * restrict errcode) {
	assert (module != NULL);
	assert ((count == 0) || (ikindhandles != NULL));
	assert (errcode != NULL);

	union Object * object;
	union Object * kind;
	int rc;
	size_t result = 0;

	if (hilbert_module_gettype(module) != HILBERT_INTERFACE_MODULE) {
		*errcode = HILBERT_ERR_INVALID_MODULE;
		goto invalid_module;
	}

	if (mtx_lock(&module->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto nolock;
	}

	rc = hilbert_module_isimmutable(module, errcode);
	if (*errcode != 0)
		goto immutable;
	if (rc) {
		*errcode = HILBERT_ERR_IMMUTABLE;
		goto immutable;
	}

	kind = hilbert_object_retrieve(module, rkindhandle, HILBERT_TYPE_KIND);
	if ((kind == NULL) || (kind->kind.type & HILBERT_TYPE_VKIND)) {
		*errcode = HILBERT_ERR_INVALID_HANDLE;
		goto wrongkind;
	}

	if (count > SIZE_MAX / sizeof(*ikindhandles)) {
		*errcode = HILBERT_ERR_NOMEM;
		goto counttoobig;
	}
	size_t ikindssize = count * sizeof(*ikindhandles);

	for (size_t i = 0; i != count; ++i) {
		kind = hilbert_object_retrieve(module, ikindhandles[i], HILBERT_TYPE_KIND);
		if (kind == NULL) {
			*errcode = HILBERT_ERR_INVALID_HANDLE;
			goto wrongkind;
		}
	}

	object = malloc(sizeof(*object));
	if (object == NULL) {
		*errcode = HILBERT_ERR_NOMEM;
		goto noobjectmem;
	}
	object->basic_functor = (struct BasicFunctor) { .type = HILBERT_TYPE_FUNCTOR, .result_kind = rkindhandle, .place_count = count, .input_kinds = NULL };

	if (ikindssize != 0) {
		object->basic_functor.input_kinds = malloc(ikindssize);
		if (object->basic_functor.input_kinds == NULL) {
			*errcode = HILBERT_ERR_NOMEM;
			goto noikindsmem;
		}
	}
	memcpy(object->basic_functor.input_kinds, ikindhandles, ikindssize);

	result = hilbert_ovector_count(module->objects);
	if (result > HILBERT_HANDLE_MAX) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto nohandle;
	}

	*errcode = hilbert_ovector_pushback(module->objects, object);
	if (*errcode != 0) {
		*errcode = HILBERT_ERR_NOMEM;
		goto noconsmem;
	}

	*errcode = hilbert_ivector_pushback(module->functorhandles, result);
	if (*errcode != 0) {
		*errcode = HILBERT_ERR_NOMEM;
		goto nohandlemem;
	}

	goto success;

nohandlemem:
	hilbert_ovector_popback(module->objects);
noconsmem:
nohandle:
	free(object->basic_functor.input_kinds);
noikindsmem:
	free(object);
noobjectmem:
wrongkind:
counttoobig:
immutable:
success:
	if (mtx_unlock(&module->mutex) != thrd_success)
		*errcode = HILBERT_ERR_INTERNAL;
nolock:
invalid_module:
	return result;
}

HilbertHandle hilbert_functor_getkind(struct HilbertModule * restrict module, HilbertHandle functorhandle, int * restrict errcode) {
	assert (module != NULL);
	assert (errcode != NULL);

	union Object * object;
	HilbertHandle result = 0;

	if (mtx_lock(&module->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto nolock;
	}

	object = hilbert_object_retrieve(module, functorhandle, HILBERT_TYPE_FUNCTOR);
	if (object == NULL) {
		*errcode = HILBERT_ERR_INVALID_HANDLE;
		goto wronghandle;
	}
	result = object->basic_functor.result_kind;

	*errcode = 0;

wronghandle:
	if (mtx_unlock(&module->mutex) != thrd_success)
		*errcode = HILBERT_ERR_INTERNAL;
nolock:
	return result;
}

HilbertHandle * hilbert_functor_getinputkinds(struct HilbertModule * restrict module, HilbertHandle functorhandle, size_t * restrict size, int * restrict errcode) {
	assert (module != NULL);
	assert (size != NULL);
	assert (errcode != NULL);

	union Object * object;
	HilbertHandle * result = NULL;

	if (mtx_lock(&module->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto nolock;
	}

	object = hilbert_object_retrieve(module, functorhandle, HILBERT_TYPE_FUNCTOR);
	if (object == NULL) {
		*errcode = HILBERT_ERR_INVALID_HANDLE;
		goto wronghandle;
	}
	*size = object->basic_functor.place_count;

	size_t resultalloc = *size * sizeof(*result);
	if (resultalloc != 0) {
		result = malloc(resultalloc);
		if (result == NULL) {
			*errcode = HILBERT_ERR_NOMEM;
			goto noresultmem;
		}
	}
	memcpy(result, object->basic_functor.input_kinds, resultalloc);

	*errcode = 0;

noresultmem:
wronghandle:
	if (mtx_unlock(&module->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		free(result);
		result = NULL;
	}
nolock:
	return result;
}
