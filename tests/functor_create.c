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

/**
 * Test to check the creation of basic functors.
 */

#include<assert.h>
#include<stdio.h>
#include<stdlib.h>

#include"hilbert.h"

int main(void) {
	int errcode;

	/* basic functors in interface modules */
	HilbertModule * imodule = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	assert (imodule != NULL);
	HilbertHandle f0 = hilbert_functor_create(imodule, 666, 0, NULL, &errcode);
	if (errcode != HILBERT_ERR_INVALID_HANDLE) {
		fprintf(stderr, "Expected invalid handle error while creating functor with non-existent result kind, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	HilbertHandle kind = hilbert_kind_create(imodule, &errcode);
	assert (errcode == 0);
	HilbertHandle vkind = hilbert_vkind_create(imodule, &errcode);
	assert (errcode == 0);
	f0 = hilbert_functor_create(imodule, vkind, 0, NULL, &errcode);
	if (errcode != HILBERT_ERR_INVALID_HANDLE) {
		fprintf(stderr, "Expected invalid handle error while creating functor with variable result kind, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	f0 = hilbert_functor_create(imodule, kind, 0, NULL, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Failed to create constant functor (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	HilbertHandle input_kinds[2];
	input_kinds[0] = 666;
	input_kinds[1] = 666;
	HilbertHandle f2 = hilbert_functor_create(imodule, kind, 2, input_kinds, &errcode);
	if (errcode != HILBERT_ERR_INVALID_HANDLE) {
		fprintf(stderr, "Expected invalid handle error while creating functor with non-existent input kinds, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	input_kinds[0] = f0;
	input_kinds[1] = f0;
	f2 = hilbert_functor_create(imodule, kind, 2, input_kinds, &errcode);
	if (errcode != HILBERT_ERR_INVALID_HANDLE) {
		fprintf(stderr, "Expected invalid handle error while creating functor with invalid input kinds, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	input_kinds[0] = kind;
	input_kinds[1] = vkind;
	f2 = hilbert_functor_create(imodule, kind, 2, input_kinds, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Failed to create 2-place functor, errcode=%d\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_module_makeimmutable(imodule);
	assert (errcode == 0);
	HilbertHandle f1 = hilbert_functor_create(imodule, kind, 1, input_kinds, &errcode);
	if (errcode != HILBERT_ERR_IMMUTABLE) {
		fprintf(stderr, "Expected immutability error attenpting to create a functor in an immutable module, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}

	/* basic functors in proof modules */
	HilbertModule * pmodule = hilbert_module_create(HILBERT_PROOF_MODULE);
	assert (pmodule != NULL);
	HilbertHandle param = hilbert_module_import(pmodule, imodule, 0, NULL, NULL, NULL, &errcode);
	assert (errcode == 0);
	kind = hilbert_object_getdesthandle(pmodule, param, kind, &errcode);
	assert (errcode == 0);
	f0 = hilbert_functor_create(pmodule, kind, 0, NULL, &errcode);
	if (errcode != HILBERT_ERR_INVALID_MODULE) {
		fprintf(stderr, "Expected invalid module error attempting to create a functor in a proof module, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}

	hilbert_module_free(imodule);
	hilbert_module_free(pmodule);
}
