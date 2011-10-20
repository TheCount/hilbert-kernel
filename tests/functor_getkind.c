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
 * Test to check functor result kinds.
 */

#include<assert.h>
#include<stdio.h>
#include<stdlib.h>

#include"hilbert.h"

int main(void) {
	int errcode;

	/* in interface modules */
	HilbertModule * imodule = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	assert (imodule != NULL);
	HilbertHandle kind = hilbert_functor_getkind(imodule, 666, &errcode);
	if (errcode != HILBERT_ERR_INVALID_HANDLE) {
		fprintf(stderr, "Expected invalid handle error attempting to obtain result kind of non-existent functor in interface module, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	kind = hilbert_kind_create(imodule, &errcode);
	assert (errcode == 0);
	HilbertHandle vkind = hilbert_vkind_create(imodule, &errcode);
	assert (errcode == 0);
	HilbertHandle input_kinds[1] = { vkind };
	HilbertHandle functor = hilbert_functor_create(imodule, kind, 1, input_kinds, &errcode);
	assert (errcode == 0);
	HilbertHandle rkind = hilbert_functor_getkind(imodule, functor, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain result kind from functor in interface module (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (rkind != kind) {
		fprintf(stderr, "Result kind of functor in interface module is not what it should be (expected: %u, got: %u)\n", (unsigned int) kind, (unsigned int) rkind);
		exit(EXIT_FAILURE);
	}

	/* in proof modules */
	HilbertModule * pmodule = hilbert_module_create(HILBERT_PROOF_MODULE);
	rkind = hilbert_functor_getkind(pmodule, 666, &errcode);
	if (errcode != HILBERT_ERR_INVALID_HANDLE) {
		fprintf(stderr, "Expected invalid handle error attempting to obtain result kind of non-existent functor in proof module, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	assert (pmodule != NULL);
	errcode = hilbert_module_makeimmutable(imodule);
	assert (errcode == 0);
	HilbertHandle param = hilbert_module_import(pmodule, imodule, 0, NULL, NULL, NULL, &errcode);
	assert (errcode == 0);
	kind = hilbert_object_getdesthandle(pmodule, param, kind, &errcode);
	assert (errcode == 0);
	vkind = hilbert_object_getdesthandle(pmodule, param, vkind, &errcode);
	assert (errcode == 0);
	functor = hilbert_object_getdesthandle(pmodule, param, functor, &errcode);
	assert (errcode == 0);
	rkind = hilbert_functor_getkind(pmodule, functor, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain result kind from functor in proof module (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (rkind != kind) {
		fprintf(stderr, "Result kind of functor in proof module is not what it should be (expected: %u, got: %u)\n", (unsigned int) kind, (unsigned int) rkind);
		exit(EXIT_FAILURE);
	}

	// FIXME: Other types of functors

	hilbert_module_free(pmodule);
	hilbert_module_free(imodule);
}
