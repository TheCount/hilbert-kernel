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
	module->ancillary = NULL;

	return module;

mutexfail:
	free(module);
allocfail:
wrongtype:
	return NULL;
}

void hilbert_module_free(struct HilbertModule * module) {
	assert (module != NULL);

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

