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

#include"cl/ivector.h"
#include"cl/ovector.h"

#include"threads/hthreads.h"

HilbertHandle hilbert_var_create(struct HilbertModule * restrict module, HilbertHandle kind, int * restrict errcode) {
	assert (module != NULL);
	assert (errcode != NULL);

	union Object * kindobject;
	union Object * object;
	int rc;
	size_t result = 0;

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

	kindobject = hilbert_object_retrieve(module, kind, HILBERT_TYPE_KIND);
	if (kindobject == NULL) {
		*errcode = HILBERT_ERR_INVALID_HANDLE;
		goto wrongkind;
	}

	object = malloc(sizeof(*object));
	if (object == NULL) {
		*errcode = HILBERT_ERR_NOMEM;
		goto novarmem;
	}
	object->var = (struct Variable) { .type = HILBERT_TYPE_VAR, .kind = kind };

	result = hilbert_ovector_count(module->objects);
	if (result > HILBERT_HANDLE_MAX) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto nohandle;
	}

	*errcode = hilbert_ovector_pushback(module->objects, object);
	if (*errcode != 0) {
		*errcode = HILBERT_ERR_NOMEM;
		goto noobjectmem;
	}

	*errcode = hilbert_ivector_pushback(module->varhandles, result);
	if (*errcode != 0) {
		*errcode = HILBERT_ERR_NOMEM;
		goto nohandlemem;
	}

	goto success;

nohandlemem:
	hilbert_ovector_popback(module->objects);
noobjectmem:
nohandle:
	free(object);
novarmem:
wrongkind:
immutable:
success:
	if (mtx_unlock(&module->mutex) != thrd_success)
		*errcode = HILBERT_ERR_INTERNAL;
nolock:
	return result;
}

HilbertHandle hilbert_var_getkind(struct HilbertModule * restrict module, HilbertHandle varhandle, int * restrict errcode) {
	assert (module != NULL);
	assert (errcode != NULL);

	HilbertHandle result = 0;
	union Object * object;

	if (mtx_lock(&module->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto nolock;
	}

	object = hilbert_object_retrieve(module, varhandle, HILBERT_TYPE_VAR);
	if (object == NULL) {
		*errcode = HILBERT_ERR_INVALID_HANDLE;
		goto wronghandle;
	}

	result = object->var.kind;
	*errcode = 0;

wronghandle:
	if (mtx_unlock(&module->mutex) != thrd_success)
		*errcode = HILBERT_ERR_INTERNAL;
nolock:
	return result;
}
