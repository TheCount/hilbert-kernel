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
 * This test checks the immutable flag of interface modules.
 */

#include<stdio.h>
#include<stdlib.h>

#include"hilbert.h"

int main(void) {
	HilbertModule * module;
	int errcode;
	int rc;

	/* Test with proof module */
	module = hilbert_module_create(HILBERT_PROOF_MODULE);
	if (module == NULL) {
		fputs("Unable to create proof module\n", stderr);
		exit(EXIT_FAILURE);
	}
	rc = hilbert_module_isimmutable(module, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Error querying mutability of proof module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (rc != 0) {
		fprintf(stderr, "Expected mutable proof module, got %d\n", rc);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_module_makeimmutable(module);
	if (errcode != HILBERT_ERR_INVALID_MODULE) {
		fprintf(stderr, "Expected invalid module error, got %d\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(module);

	/* Test with interface module */
	module = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	if (module == NULL) {
		fputs("Unable to create interface module\n", stderr);
		exit(EXIT_FAILURE);
	}
	rc = hilbert_module_isimmutable(module, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Error querying mutability of interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (rc != 0) {
		fprintf(stderr, "Expected mutable interface module, got %d\n", rc);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_module_makeimmutable(module);
	if (errcode != 0) {
		fprintf(stderr, "Unable to make interface module immutable (return code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	rc = hilbert_module_isimmutable(module, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Error querying mutability of interface module (error code=%d)\n", errcode);
		exit (EXIT_FAILURE);
	}
	if (!rc) {
		fprintf(stderr, "Interface module is not immutable after call to hilbert_module_makeimmutable() (return code=%d)\n", rc);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_module_makeimmutable(module);
	if (errcode != HILBERT_ERR_IMMUTABLE) {
		fprintf(stderr, "Expected error making interface module immutable twice, got %d\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(module);
}
