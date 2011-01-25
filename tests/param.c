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
 * Test to check interface parameterisation.
 */

#include<stdio.h>
#include<stdlib.h>

#include"hilbert.h"

int main(void) {
	HilbertModule * src;
	HilbertModule * dest;
	HilbertHandle param;
	int errcode;

	/* test with wrong kinds of modules */
	src = hilbert_module_create(HILBERT_PROOF_MODULE);
	dest = hilbert_module_create(HILBERT_PROOF_MODULE);
	if ((src == NULL) || (dest == NULL)) {
		fputs("Unable to create modules (src=proof, dest=proof)\n", stderr);
		exit(EXIT_FAILURE);
	}
	param = hilbert_module_param(dest, src, 0, NULL, NULL, &errcode);
	if (errcode != HILBERT_ERR_INVALID_MODULE) {
		fprintf(stderr, "Expected invalid module error (src=proof, dest=proof), got %d\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(dest);
	dest = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	if (dest == NULL) {
		fputs("Unable to create module (src=proof, dest=interface)\n", stderr);
		exit(EXIT_FAILURE);
	}
	param = hilbert_module_param(dest, src, 0, NULL, NULL, &errcode);
	if (errcode != HILBERT_ERR_INVALID_MODULE) {
		fprintf(stderr, "Expected invalid module error (src=proof, dest=interface), got %d\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(src);
	hilbert_module_free(dest);
	src = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	dest = hilbert_module_create(HILBERT_PROOF_MODULE);
	if ((src == NULL) || (dest == NULL)) {
		fputs("Unable to create modules (src=interface, dest=proof)\n", stderr);
		exit(EXIT_FAILURE);
	}
	param = hilbert_module_param(dest, src, 0, NULL, NULL, &errcode);
	if (errcode != HILBERT_ERR_INVALID_MODULE) {
		fprintf(stderr, "Expected invalid module error (src=interface, dest=proof), got %d\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(src);
	hilbert_module_free(dest);

	/* test with wrong kind of mutability */
	src = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	dest = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	if ((src == NULL) || (dest == NULL)) {
		fputs("Unable to create modules (src=interface, dest=interface)\n", stderr);
		exit(EXIT_FAILURE);
	}
	param = hilbert_module_param(dest, src, 0, NULL, NULL, &errcode);
	if (errcode != HILBERT_ERR_IMMUTABLE) {
		fprintf(stderr, "Expected mutability error (src=mutable, dest=mutable), got %d\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_module_makeimmutable(dest);
	if (errcode != 0) {
		fprintf(stderr, "Unable to make destination module immutable (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	param = hilbert_module_param(dest, src, 0, NULL, NULL, &errcode);
	if (errcode != HILBERT_ERR_IMMUTABLE) {
		fprintf(stderr, "Expected mutability error (src=mutable, dest=immutable), got %d\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_module_makeimmutable(src);
	if (errcode != 0) {
		fprintf(stderr, "Unable to make source module immutable (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	param = hilbert_module_param(dest, src, 0, NULL, NULL, &errcode);
	if (errcode != HILBERT_ERR_IMMUTABLE) {
		fprintf(stderr, "Expected mutability error (src=immutable, dest=immutable), got %d\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(src);
	hilbert_module_free(dest);

	/* test empty param */
	src = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	dest = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	if ((src == NULL) || (dest == NULL)) {
		fputs("Unable to create modules (src=interface, dest=interface)\n", stderr);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_module_makeimmutable(src);
	if (errcode != 0) {
		fprintf(stderr, "Unable to make source module immutable (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	param = hilbert_module_param(dest, src, 0, NULL, NULL, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to parameterise module (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(src);
	hilbert_module_free(dest);

	// FIXME: add multi-module test with mapper
}
