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
 * Test to check obtaining of input kinds of functors.
 */

#include<assert.h>
#include<stdio.h>
#include<stdlib.h>

#include"hilbert.h"

int main(void) {
	int errcode;
	size_t size;

	/* in interface modules */
	HilbertModule * imodule = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	assert (imodule != NULL);
	HilbertHandle * ikinds = hilbert_functor_getinputkinds(imodule, 666, &size, &errcode);
	if (errcode != HILBERT_ERR_INVALID_HANDLE) {
		fprintf(stderr, "Expected invalid handle error while obtaining input kinds of non-existent functor in interface module, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	HilbertHandle kind = hilbert_kind_create(imodule, &errcode);
	assert (errcode == 0);
	HilbertHandle vkind = hilbert_vkind_create(imodule, &errcode);
	assert (errcode == 0);
	HilbertHandle f0 = hilbert_functor_create(imodule, kind, 0, NULL, &errcode);
	assert (errcode == 0);
	ikinds = hilbert_functor_getinputkinds(imodule, f0, &size, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain input kinds from constant functor in interface module (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (size != 0) {
		fprintf(stderr, "Expected constant functor in interface module to have zero input kinds, received %zu\n", size);
		exit(EXIT_FAILURE);
	}
	hilbert_harray_free(ikinds);
	HilbertHandle ikindarray[2] = { kind, vkind };
	HilbertHandle f2 = hilbert_functor_create(imodule, kind, 2, ikindarray, &errcode);
	assert (errcode == 0);
	ikinds = hilbert_functor_getinputkinds(imodule, f2, &size, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain input kinds from functor in interface module (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (size != 2) {
		fprintf(stderr, "Wrong number of input kinds from functor in interface module (expected: 2, got: %zu)\n", size);
		exit(EXIT_FAILURE);
	}
	if ((ikinds[0] != kind) || (ikinds[1] != vkind)) {
		fprintf(stderr, "Got back wrong input kinds in interface module (expected: {%u, %u}, got: {%u, %u})\n", (unsigned int) kind, (unsigned int) vkind, (unsigned int) ikinds[0], (unsigned int) ikinds[1]);
		exit(EXIT_FAILURE);
	}
	hilbert_harray_free(ikinds);

	/* in proof modules */
	HilbertModule * pmodule = hilbert_module_create(HILBERT_PROOF_MODULE);
	assert (pmodule != NULL);
	errcode = hilbert_module_makeimmutable(imodule);
	assert (errcode == 0);
	HilbertHandle param = hilbert_module_import(pmodule, imodule, 0, NULL, NULL, NULL, &errcode);
	assert (errcode == 0);
	kind = hilbert_object_getdesthandle(pmodule, param, kind, &errcode);
	assert (errcode == 0);
	vkind = hilbert_object_getdesthandle(pmodule, param, vkind, &errcode);
	assert (errcode == 0);
	f0 = hilbert_object_getdesthandle(pmodule, param, f0, &errcode);
	assert (errcode == 0);
	f2 = hilbert_object_getdesthandle(pmodule, param, f2, &errcode);
	assert (errcode == 0);
	ikinds = hilbert_functor_getinputkinds(pmodule, f0, &size, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain input kinds from constant functor in proof module (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (size != 0) {
		fprintf(stderr, "Expected constant functor in proof module to have zero input kinds, received %zu\n", size);
		exit(EXIT_FAILURE);
	}
	hilbert_harray_free(ikinds);
	ikinds = hilbert_functor_getinputkinds(pmodule, f2, &size, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain input kinds from functor in proof module (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (size != 2) {
		fprintf(stderr, "Wrong number of input kinds from functor in proof module (expected: 2, got: %zu)\n", size);
		exit(EXIT_FAILURE);
	}
	if ((ikinds[0] != kind) || (ikinds[1] != vkind)) {
		fprintf(stderr, "Got back wrong input kinds in proof module (expected: {%u, %u}, got: {%u, %u})\n", (unsigned int) kind, (unsigned int) vkind, (unsigned int) ikinds[0], (unsigned int) ikinds[1]);
		exit(EXIT_FAILURE);
	}

	hilbert_module_free(imodule);
	hilbert_module_free(pmodule);
	hilbert_harray_free(ikinds);

	// FIXME: Other types of functors
}
