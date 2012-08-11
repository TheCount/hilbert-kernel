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
 * Test to check variable kind equivalence.
 */

#include<assert.h>
#include<stdio.h>
#include<stdlib.h>

#include"hilbert.h"

#define NUM_HANDLES 5

/**
 * Checks whether two variable kind handles in an array denote equivalent variable kinds.
 * If the result is not as expected, the program is terminated indicating failure.
 *
 * @param module Pointer to Hilbert module to check equivalence in.
 * @param handles Pointer to array of kind handles.
 * @param i1 Index of first kind handle in <code>handles</code> to check.
 * @param i2 Index of second kind handle in <code>handles</code> to check.
 * @param expected Expected result of equivalence check.
 */
void check_eq(HilbertModule * restrict module, HilbertHandle * restrict handles, size_t i1, size_t i2, int expected) {
	assert (module != NULL);
	assert (handles != NULL);
	int rc;
	int errcode;
	rc = hilbert_kind_isequivalent(module, handles[i1], handles[i2], &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to check variable kinds %zu and %zu for equivalence (error code=%d)\n", i1, i2, errcode);
		exit(EXIT_FAILURE);
	}
	if (rc && (!expected)) {
		fprintf(stderr, "Expected variable kinds %zu and %zu to be inequivalent\n", i1, i2);
		exit(EXIT_FAILURE);
	}
	if ((!rc) && (expected)) {
		fprintf(stderr, "Expected variable kinds %zu and %zu to be equivalent\n", i1, i2);
		exit(EXIT_FAILURE);
	}
}

void check_eq_matrix(HilbertModule * restrict module, HilbertHandle * restrict handles,
		int expected[NUM_HANDLES][NUM_HANDLES]) {
	for (size_t i = 0; i != NUM_HANDLES; ++i)
		for (size_t j = 0; j != NUM_HANDLES; ++j)
			check_eq(module, handles, i, j, expected[i][j]);
}

int main(void) {
	HilbertModule * module;
	HilbertHandle handles[NUM_HANDLES];
	int errcode;

	module = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	if (module == NULL) {
		fputs("Unable to create Hilbert interface module\n", stderr);
		exit(EXIT_FAILURE);
	}
	hilbert_kind_isequivalent(module, 666, 666, &errcode);
	if (errcode != HILBERT_ERR_INVALID_HANDLE) {
		fprintf(stderr, "Expected invalid handle error, got %d\n", errcode);
		exit(EXIT_FAILURE);
	}
	assert (NUM_HANDLES >= 5);
	handles[0] = hilbert_vkind_create(module, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create vkind0 in Hilbert interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	handles[1] = hilbert_kind_alias(module, handles[0], &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to alias vkind0 as vkind1 in Hilbert interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	handles[2] = hilbert_vkind_create(module, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create vkind2 in Hilbert interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	handles[3] = hilbert_vkind_create(module, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create vkind3 in Hilbert interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	handles[4] = hilbert_vkind_create(module, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create vkind4 in Hilbert interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}

	/* check expected equivalences at this point */
	int expected1[NUM_HANDLES][NUM_HANDLES] = {
		{ 1, 1, 0, 0, 0 },
		{ 1, 1, 0, 0, 0 },
		{ 0, 0, 1, 0, 0 },
		{ 0, 0, 0, 1, 0 },
		{ 0, 0, 0, 0, 1 }
	};
	check_eq_matrix(module, handles, expected1);

	/* 3 <-> 4 */
	errcode = hilbert_kind_identify(module, handles[3], handles[4]);
	if (errcode != 0) {
		fprintf(stderr, "Unable to identify variable kinds 3, 4 in Hilbert interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	int expected2[NUM_HANDLES][NUM_HANDLES] = {
		{ 1, 1, 0, 0, 0 },
		{ 1, 1, 0, 0, 0 },
		{ 0, 0, 1, 0, 0 },
		{ 0, 0, 0, 1, 1 },
		{ 0, 0, 0, 1, 1 }
	};
	check_eq_matrix(module, handles, expected2);

	/* {0, 1} <-> 2 */
	errcode = hilbert_kind_identify(module, handles[2], handles[0]);
	if (errcode != 0) {
		fprintf(stderr, "Unable to identify variable kinds {0, 1}, 2 in Hilbert interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	int expected3[NUM_HANDLES][NUM_HANDLES] = {
		{ 1, 1, 1, 0, 0 },
		{ 1, 1, 1, 0, 0 },
		{ 1, 1, 1, 0, 0 },
		{ 0, 0, 0, 1, 1 },
		{ 0, 0, 0, 1, 1 }
	};
	check_eq_matrix(module, handles, expected3);

	/* {0, 1, 2} <-> {3, 4} */
	errcode = hilbert_kind_identify(module, handles[1], handles[4]);
	if (errcode != 0) {
		fprintf(stderr, "Unable to identify variable kinds {0, 1, 2}, {3, 4} in Hilbert interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	int expected4[NUM_HANDLES][NUM_HANDLES] = {
		{ 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1 },
		{ 1, 1, 1, 1, 1 }
	};
	check_eq_matrix(module, handles, expected4);

	/* Note: test for proof modules is contained in import.c */
	
	hilbert_module_free(module);
}
