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
 * Test to check kind identification.
 */

#include<stdio.h>
#include<stdlib.h>

#include"hilbert.h"

int main(void) {
	HilbertModule * module;
	HilbertHandle kind1, kind2, kind3, kind4, kind5;
	int errcode;

	// FIXME: Also check ID failure in proof modules through imports

	/*** Id test in interface modules ***/

	module = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	if (module == NULL) {
		fputs("Unable to create Hilbert interface module\n", stderr);
		exit(EXIT_FAILURE);
	}
	/* invalid handle */
	errcode = hilbert_kind_identify(module, 666, 666);
	if (errcode != HILBERT_ERR_INVALID_HANDLE) {
		fprintf(stderr, "Expected invalid handle error, got %d\n", errcode);
		exit(EXIT_FAILURE);
	}
	/* 1 <-> 1 */
	kind1 = hilbert_kind_create(module, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create kind1 in Hilbert interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_kind_identify(module, kind1, kind1);
	if (errcode != 0) {
		fprintf(stderr, "Error identifying equal kinds (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_kind_identify(module, kind1, kind1);
	if (errcode != 0) {
		fprintf(stderr, "Error identifying equal kinds twice (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	/* 2 <-> 3 */
	kind2 = hilbert_kind_create(module, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create kind2 in Hilbert interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	kind3 = hilbert_kind_create(module, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create kind3 in Hilbert interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_kind_identify(module, kind2, kind3);
	if (errcode != 0) {
		fprintf(stderr, "Error identifying kinds 2, 3 in Hilbert interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_kind_identify(module, kind3, kind2);
	if (errcode != 0) {
		fprintf(stderr, "Error identifying kinds 2, 3 in Hilbert interface module twice (error code=%d)\n",
				errcode);
		exit(EXIT_FAILURE);
	}
	/* 1 <-> {2, 3} */
	errcode = hilbert_kind_identify(module, kind3, kind1);
	if (errcode != 0) {
		fprintf(stderr, "Error identifying kinds 1, {2, 3} in Hilbert interface module (error code=%d)\n",
				errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_kind_identify(module, kind1, kind2);
	if (errcode != 0) {
		fprintf(stderr, "Error identifying kinds 1, {2, 3} in Hilbert interface module twice (error code=%d)\n",
				errcode);
		exit(EXIT_FAILURE);
	}
	/* 4 <-> 5 */
	kind4 = hilbert_kind_create(module, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create kind4 in Hilbert interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	kind5 = hilbert_kind_create(module, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create kind5 in Hilbert interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_kind_identify(module, kind5, kind4);
	if (errcode != 0) {
		fprintf(stderr, "Error identifying kinds 4, 5 in Hilbert interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_kind_identify(module, kind5, kind4);
	if (errcode != 0) {
		fprintf(stderr, "Error identifying kinds 4, 5 in Hilbert interface module twice (error code=%d)\n",
				errcode);
		exit(EXIT_FAILURE);
	}
	/* {1, 2, 3} <-> {4, 5} */
	errcode = hilbert_kind_identify(module, kind3, kind4);
	if (errcode != 0) {
		fprintf(stderr, "Error identifying kinds {1, 2, 3}, {4, 5} in Hilbert interface module (error code=%d)\n",
				errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_kind_identify(module, kind5, kind1);
	if (errcode != 0) {
		fprintf(stderr, "Error identifying kinds {1, 2, 3}, {4, 5} in Hilbert interface module twice "
				"(error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	/* immutable */
	errcode = hilbert_module_makeimmutable(module);
	if (errcode != 0) {
		fprintf(stderr, "Unable to make Hilbert interface module immutable (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_kind_identify(module, kind1, kind5);
	if (errcode != HILBERT_ERR_IMMUTABLE) {
		fprintf(stderr, "Expected immutability error, got %d\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(module);
}
