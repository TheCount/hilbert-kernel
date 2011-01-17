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

#include<stdlib.h>

#include"cl/ivector.h"
#include"cl/ovector.h"

#include"threads/hthreads.h"

HilbertHandle hilbert_kind_create(struct HilbertModule * restrict module, int * restrict errcode) {
	assert (module != NULL);
	assert (errcode != NULL);

	union Object * object;
	int rc;
	size_t result;

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

	object = malloc(sizeof(*object));
	if (object == NULL) {
		*errcode = HILBERT_ERR_NOMEM;
		goto nokindmem;
	}
	object->kind.type = HILBERT_TYPE_KIND;

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

	*errcode = hilbert_ivector_pushback(module->kindhandles, result);
	if (*errcode != 0) {
		*errcode = HILBERT_ERR_NOMEM;
		goto nohandlemem;
	}

	goto success;

nohandlemem:
	hilbert_ovector_popback(module->objects);
noconsmem:
nohandle:
	free(object);
nokindmem:
immutable:
success:
	if (mtx_unlock(&module->mutex) != thrd_success)
		*errcode = HILBERT_ERR_INTERNAL;
nolock:
invalid_module:
	return result;
}
