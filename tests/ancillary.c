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
 * Test to check the ancillary data facilities of a Hilbert module.
 */

#include<stdio.h>
#include<stdlib.h>

#include"hilbert.h"

int main(void) {
	HilbertModule * module;
	void * newdata;
	void * olddata;
	int errcode;

	module = hilbert_module_create(HILBERT_PROOF_MODULE);
	if (module == NULL) {
		fputs("Unable to create Hilbert module\n", stderr);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_module_getancillary(module, &olddata);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain ancillary data from Hilbert module (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (olddata != NULL) {
		fprintf(stderr, "Expected ancillary data to be NULL by default, got %p\n", olddata);
		exit(EXIT_FAILURE);
	}
	newdata = fprintf;
	errcode = hilbert_module_setancillary(module, newdata, &olddata);
	if (errcode != 0) {
		fprintf(stderr, "Unable to set ancillary data of Hilbert module (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (olddata != NULL) {
		fprintf(stderr, "Expected old ancillary data to be NULL before first assignment, got %p\n", olddata);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_module_getancillary(module, &olddata);
	if (errcode != 0) {
		fprintf(stderr, "Unable to get freshly set ancillary data of Hilbert module (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (olddata != newdata) {
		fprintf(stderr, "Set and obtained ancillary data of Hilbert module do not match (set=%p, obtained=%p)\n", newdata, olddata);
		exit(EXIT_FAILURE);
	}
	newdata = fputs;
	errcode = hilbert_module_setancillary(module, newdata, NULL);
	if (errcode != 0) {
		fprintf(stderr, "Unable to set ancillary data of Hilbert module with no return of old data (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_module_getancillary(module, &olddata);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain ancillary data of Hilbert module previuously set with no return of old data (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (newdata != olddata) {
		fprintf(stderr, "Set (with no return of old data) and obtained ancillary data of Hilbert module do not match (set=%p, obtained=%p)\n", newdata, olddata);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(module);
}
