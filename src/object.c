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

#include"cl/pmap.h"
#include"cl/ovector.h"

#include"threads/hthreads.h"

HilbertHandle * hilbert_module_getobjects(struct HilbertModule * restrict module, size_t * restrict size,
		int * restrict errcode) {
	assert (module != NULL);
	assert (size != NULL);
	assert (errcode != NULL);

	HilbertHandle * result = NULL;

	if (mtx_lock(&module->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto nolock;
	}

	*size = hilbert_ovector_count(module->objects);
	result = malloc(*size * sizeof(*result));
	if (result == NULL) {
		*errcode = HILBERT_ERR_NOMEM;
		goto nomem;
	}

	for (size_t i = 0; i != *size; ++i)
		result[i] = i;

	*errcode = 0;

nomem:
	if (mtx_unlock(&module->mutex) != thrd_success)
		*errcode = HILBERT_ERR_INTERNAL;
nolock:
	return result;
}

unsigned int hilbert_object_gettype(struct HilbertModule * restrict module, HilbertHandle handle, int * restrict errcode) {
	assert (module != NULL);
	assert (errcode != NULL);

	unsigned int type = 0;

	if (mtx_lock(&module->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto nolock;
	}

	union Object * object = hilbert_object_retrieve(module, handle, ~0U);
	if (object == NULL) {
		*errcode = HILBERT_ERR_INVALID_HANDLE;
		goto invalidhandle;
	}
	type = object->generic.type;
	*errcode = 0;

invalidhandle:
	if (mtx_unlock(&module->mutex) != thrd_success)
		*errcode = HILBERT_ERR_INTERNAL;
nolock:
	return type;
}

HilbertHandle hilbert_object_getparam(struct HilbertModule * restrict module, HilbertHandle handle,
		int * restrict errcode) {
	assert (module != NULL);
	assert (errcode != NULL);

	HilbertHandle result = 0;

	if (mtx_lock(&module->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto nolock;
	}

	union Object * object = hilbert_object_retrieve(module, handle, HILBERT_TYPE_EXTERNAL);
	if (object == NULL) {
		*errcode = HILBERT_ERR_INVALID_HANDLE;
		goto wronghandle;
	}

	if (object->generic.type & HILBERT_TYPE_KIND) {
		result = hilbert_ivector_get(module->paramhandles, object->external_kind.paramindex);
	} else { // FIXME: functors, etc.
		assert (0);
	}

	*errcode = 0;

wronghandle:
	if (mtx_unlock(&module->mutex) != thrd_success)
		*errcode = HILBERT_ERR_INTERNAL;
nolock:
	return result;
}

HilbertModule * hilbert_object_getsource(struct HilbertModule * restrict module, HilbertHandle handle,
		int * restrict errcode) {
	assert (module != NULL);
	assert (errcode != NULL);

	HilbertModule * result = NULL;

	HilbertHandle param = hilbert_object_getparam(module, handle, errcode);
	if (*errcode != 0)
		goto noparam;

	if (mtx_lock(&module->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto nolock;
	}

	union Object * object = hilbert_object_retrieve(module, param, HILBERT_TYPE_PARAM);
	assert (object != NULL);

	result = object->param.module;
	*errcode = 0;

	if (mtx_unlock(&module->mutex) != thrd_success)
		*errcode = HILBERT_ERR_INTERNAL;

nolock:
noparam:
	return result;
}

HilbertHandle hilbert_object_getsourcehandle(struct HilbertModule * restrict module, HilbertHandle handle,
		int * restrict errcode) {
	assert (module != NULL);
	assert (errcode != NULL);

	HilbertHandle result = 0;

	HilbertHandle param = hilbert_object_getparam(module, handle, errcode);
	if (*errcode != 0)
		goto noparam;

	if (mtx_lock(&module->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto nolock;
	}

	union Object * object = hilbert_object_retrieve(module, param, HILBERT_TYPE_PARAM);
	assert (object != NULL);

	result = *hilbert_pmap_post(object->param.handle_map, handle);
	*errcode = 0;

	if (mtx_unlock(&module->mutex) != thrd_success)
		*errcode = HILBERT_ERR_INTERNAL;

nolock:
noparam:
	return result;
}

HilbertHandle hilbert_object_getdesthandle(struct HilbertModule * restrict module, HilbertHandle paramhandle,
		HilbertHandle handle, int * restrict errcode) {
	assert (module != NULL);
	assert (errcode != NULL);

	HilbertHandle result = 0;

	if (mtx_lock(&module->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto nolock;
	}

	union Object * param = hilbert_object_retrieve(module, paramhandle, HILBERT_TYPE_PARAM);
	if (param == NULL) {
		*errcode = HILBERT_ERR_INVALID_HANDLE;
		goto noparam;
	}

	const HilbertHandle * resultp = hilbert_pmap_pre(param->param.handle_map, handle);
	if (resultp == NULL) {
		*errcode = HILBERT_ERR_INVALID_HANDLE;
		goto nohandle;
	}

	result = *resultp;
	*errcode = 0;

nohandle:
noparam:
	if (mtx_unlock(&module->mutex) != thrd_success)
		*errcode = HILBERT_ERR_INTERNAL;
nolock:
	return result;
}

