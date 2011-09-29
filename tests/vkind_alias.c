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
 * Test to check variable kind aliasing.
 */

#include<stdio.h>
#include<stdlib.h>

#include"hilbert.h"

int main(void) {
	int errcode;
	int rc;

	/* aliasing in interface modules */
	HilbertModule * module = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	if (module == NULL) {
		fputs("Unable to create Hilbert interface module\n", stderr);
		exit(EXIT_FAILURE);
	}
	HilbertHandle vkind = hilbert_vkind_create(module, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create variable kind in interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	HilbertHandle alias = hilbert_kind_alias(module, 666, &errcode);
	if (errcode != HILBERT_ERR_INVALID_HANDLE) {
		fprintf(stderr, "Expected invalid handle error, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	alias = hilbert_kind_alias(module, vkind, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create alias kind in interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	rc = hilbert_kind_isequivalent(module, vkind, alias, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to check variable kind and its alias for equivalence (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (!rc) {
		fputs("Expected variable kind and its alias to be equivalent\n", stderr);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_module_makeimmutable(module);
	if (errcode != 0) {
		fprintf(stderr, "Unable to make interface module immutable (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	alias = hilbert_kind_alias(module, vkind, &errcode);
	if (errcode != HILBERT_ERR_IMMUTABLE) {
		fprintf(stderr, "Expected immutability error, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(module);

	/* aliasing in proof modules */
	HilbertModule * src = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	HilbertModule * dest = hilbert_module_create(HILBERT_PROOF_MODULE);
	if ((src == NULL) || (dest == NULL)) {
		fputs("Unable to create Hilbert modules\n", stderr);
		exit(EXIT_FAILURE);
	}
	HilbertHandle svkind = hilbert_vkind_create(src, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create variable kind in src (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_module_makeimmutable(src);
	if (errcode != 0) {
		fprintf(stderr, "Unable to make Hilbert module src immutable (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	HilbertHandle param = hilbert_module_import(dest, src, 0, NULL, NULL, NULL, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to import src into dest (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	HilbertHandle dkind = hilbert_object_getdesthandle(dest, param, svkind, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain dest kind (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	alias = hilbert_kind_alias(dest, dkind, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to alias dest kind (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	rc = hilbert_kind_isequivalent(dest, dkind, alias, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to check dest kind and alias for equivalence (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (!rc) {
		fputs("Expected dest kind and its alias to be equivalent\n", stderr);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(dest);
	hilbert_module_free(src);
}
