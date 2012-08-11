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
 * Test to check kind creation.
 */

#include<stdio.h>
#include<stdlib.h>

#include"hilbert.h"

int main(void) {
	HilbertModule * module;
	HilbertHandle kind;
	int errcode;

	module = hilbert_module_create(HILBERT_PROOF_MODULE);
	if (module == NULL) {
		fputs("Unable to create Hilbert proof module\n", stderr);
		exit(EXIT_FAILURE);
	}
	kind = hilbert_kind_create(module, &errcode);
	if (errcode != HILBERT_ERR_INVALID_MODULE) {
		fprintf(stderr, "Expected invalid module error, got %d\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(module);

	module = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	if (module == NULL) {
		fputs("Unable to create Hilbert interface module\n", stderr);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_module_makeimmutable(module);
	if (errcode != 0) {
		fprintf(stderr, "Unable to make Hilbert interface module immutable (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	kind = hilbert_kind_create(module, &errcode);
	if (errcode != HILBERT_ERR_IMMUTABLE) {
		fprintf(stderr, "Expected immutable module error, got %d\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(module);

	module = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	if (module == NULL) {
		fputs("Unable to create Hilbert interface module\n", stderr);
		exit(EXIT_FAILURE);
	}
	kind = hilbert_kind_create(module, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create kind in interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(module);
}
