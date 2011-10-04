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
 * Test to check variable creation.
 */

#include<assert.h>
#include<stdio.h>
#include<stdlib.h>

#include"hilbert.h"

int main(void) {
	int errcode;

	/* variables in interface modules */
	HilbertModule * imodule = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	assert (imodule != NULL);
	HilbertHandle var = hilbert_var_create(imodule, 666, &errcode);
	if (errcode != HILBERT_ERR_INVALID_HANDLE) {
		fprintf(stderr, "Expected invalid handle error with nonexistent handle, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	HilbertHandle kind = hilbert_kind_create(imodule, &errcode);
	assert (errcode == 0);
	var = hilbert_var_create(imodule, kind, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create variable (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	HilbertHandle vkind = hilbert_vkind_create(imodule, &errcode);
	assert (errcode == 0);
	HilbertHandle var2 = hilbert_var_create(imodule, vkind, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create variable of variable kind (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_module_makeimmutable(imodule);
	assert (errcode == 0);
	HilbertHandle var3 = hilbert_var_create(imodule, kind, &errcode);
	if (errcode != HILBERT_ERR_IMMUTABLE) {
		fprintf(stderr, "Expected immutability error, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}

	/* variables in proof modules */
	HilbertModule * pmodule = hilbert_module_create(HILBERT_PROOF_MODULE);
	assert (pmodule != NULL);
	var = hilbert_var_create(pmodule, 666, &errcode);
	if (errcode != HILBERT_ERR_INVALID_HANDLE) {
		fprintf(stderr, "Expected invalid handle error with nonexistent handle, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	HilbertHandle param = hilbert_module_import(pmodule, imodule, 0, NULL, NULL, NULL, &errcode);
	assert (errcode == 0);
	kind = hilbert_object_getdesthandle(pmodule, param, kind, &errcode);
	assert (errcode == 0);
	vkind = hilbert_object_getdesthandle(pmodule, param, vkind, &errcode);
	assert (errcode == 0);
	var = hilbert_var_create(pmodule, kind, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create variable (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	var2 = hilbert_var_create(pmodule, vkind, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create variable of variable kind (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}

	hilbert_module_free(imodule);
	hilbert_module_free(pmodule);
}
