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

#include"threads/hthreads.h"

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
