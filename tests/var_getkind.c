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
 * Test to check obtaining kinds of variables.
 */

#include<assert.h>
#include<stdio.h>
#include<stdlib.h>

#include"hilbert.h"

int main(void) {
	int errcode;

	/* In interface modules */
	HilbertModule * imodule = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	assert (imodule != NULL);
	HilbertHandle rkind = hilbert_var_getkind(imodule, 666, &errcode);
	if (errcode != HILBERT_ERR_INVALID_HANDLE) {
		fprintf(stderr, "Expected invalid handle error trying to get kind of nonexistent variable, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	HilbertHandle kind = hilbert_kind_create(imodule, &errcode);
	assert (errcode == 0);
	HilbertHandle vkind = hilbert_vkind_create(imodule, &errcode);
	assert (errcode == 0);
	HilbertHandle var1 = hilbert_var_create(imodule, kind, &errcode);
	assert (errcode == 0);
	HilbertHandle var2 = hilbert_var_create(imodule, vkind, &errcode);
	assert (errcode == 0);
	rkind = hilbert_var_getkind(imodule, var1, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain kind of variable (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (rkind != kind) {
		fprintf(stderr, "Received wrong kind from hilbert_var_getkind() (expected: %u, got %u)\n", (unsigned int) kind, (unsigned int) rkind);
		exit(EXIT_FAILURE);
	}
	rkind = hilbert_var_getkind(imodule, var2, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain variable kind of variable (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (rkind != vkind) {
		fprintf(stderr, "Received wrong variable kind from hilbert_var_getkind() (expected: %u, got %u)\n", (unsigned int) vkind, (unsigned int) rkind);
		exit(EXIT_FAILURE);
	}

	/* In proof modules */
	errcode = hilbert_module_makeimmutable(imodule);
	assert (errcode == 0);
	HilbertModule * pmodule = hilbert_module_create(HILBERT_PROOF_MODULE);
	assert (pmodule != NULL);
	rkind = hilbert_var_getkind(pmodule, 666, &errcode);
	if (errcode != HILBERT_ERR_INVALID_HANDLE) {
		fprintf(stderr, "Expected invalid handle error trying to get kind of nonexistent variable, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	HilbertHandle param = hilbert_module_import(pmodule, imodule, 0, NULL, NULL, NULL, &errcode);
	assert (errcode == 0);
	kind = hilbert_object_getdesthandle(pmodule, param, kind, &errcode);
	assert (errcode == 0);
	vkind = hilbert_object_getdesthandle(pmodule, param, vkind, &errcode);
	assert (errcode == 0);
	var1 = hilbert_var_create(pmodule, kind, &errcode);
	assert (errcode == 0);
	var2 = hilbert_var_create(pmodule, vkind, &errcode);
	assert (errcode == 0);
	rkind = hilbert_var_getkind(pmodule, var1, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain kind of variable (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (rkind != kind) {
		fprintf(stderr, "Received wrong kind from hilbert_var_getkind() (expected: %u, got %u)\n", (unsigned int) kind, (unsigned int) rkind);
		exit(EXIT_FAILURE);
	}
	rkind = hilbert_var_getkind(pmodule, var2, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain variable kind of variable (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (rkind != vkind) {
		fprintf(stderr, "Received wrong variable kind from hilbert_var_getkind() (expected: %u, got %u)\n", (unsigned int) vkind, (unsigned int) rkind);
		exit(EXIT_FAILURE);
	}

	hilbert_module_free(imodule);
	hilbert_module_free(pmodule);
}
