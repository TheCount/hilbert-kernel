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
 * Variable kind equivalence class checker.
 */

#include<stdio.h>
#include<stdlib.h>

#include"hilbert.h"

int main(void) {
	HilbertModule * module;
	HilbertHandle vkind1, vkind2, vkind3;
	HilbertHandle * eqc;
	int errcode;
	size_t count;

	module = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	if (module == NULL) {
		fputs("Unable to create Hilbert interface module\n", stderr);
		exit(EXIT_FAILURE);
	}
	eqc = hilbert_kind_equivalenceclass(module, 666, &count, &errcode);
	if ((errcode != HILBERT_ERR_INVALID_HANDLE) || (eqc != NULL)) {
		fprintf(stderr, "Expected invalid handle error and NULL result (error code=%d, result=%p)\n", errcode, (void *) eqc);
		exit(EXIT_FAILURE);
	}
	/* singleton */
	vkind1 = hilbert_vkind_create(module, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create vkind1 in Hilbert interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	eqc = hilbert_kind_equivalenceclass(module, vkind1, &count, &errcode);
	if ((errcode != 0) || (eqc == NULL)) {
		fprintf(stderr, "Unable to create vkind1 equivalence class (error code=%d, result=%p)\n", errcode, (void *) eqc);
		exit(EXIT_FAILURE);
	}
	if (count != 1) {
		fprintf(stderr, "Expected vkind1 equivalence class to have size 1, got %zu\n", count);
		exit(EXIT_FAILURE);
	}
	if (eqc[0] != vkind1) {
		fprintf(stderr, "Expected equivalence class to contain vkind1, got { %u }\n", (unsigned int) eqc[0]);
		exit(EXIT_FAILURE);
	}
	hilbert_array_free(eqc);
	/* aliased kinds */
	vkind2 = hilbert_vkind_create(module, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create vkind2 in Hilbert interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	vkind3 = hilbert_kind_alias(module, vkind2, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to alias vkind2 to vkind3 in Hilbert interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	eqc = hilbert_kind_equivalenceclass(module, vkind3, &count, &errcode);
	if ((errcode != 0) || (eqc == NULL)) {
		fprintf(stderr, "Unable to create variable kind 2, 3 equivalence class (error code=%d, result=%p)\n", errcode, (void *) eqc);
		exit(EXIT_FAILURE);
	}
	if (count != 2) {
		fprintf(stderr, "Expected variable kind 2, 3 equivalence class to have size 2, got %zu\n", count);
		exit(EXIT_FAILURE);
	}
	if (((vkind2 != eqc[0]) && (vkind2 != eqc[1])) || ((vkind3 != eqc[0]) && (vkind3 != eqc[1]))) {
		fprintf(stderr, "Expected equivalence class to contain variable kinds 2, 3, got { %u, %u }\n",
				(unsigned int) eqc[0], (unsigned int) eqc[1]);
		exit(EXIT_FAILURE);
	}
	hilbert_array_free(eqc);
	/* all together now */
	errcode = hilbert_kind_identify(module, vkind1, vkind2);
	if (errcode != 0) {
		fprintf(stderr, "Unable to identify variable kind 1, {2, 3} in Hilbert inteface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	eqc = hilbert_kind_equivalenceclass(module, vkind2, &count, &errcode);
	if ((errcode != 0) || (eqc == NULL)) {
		fprintf(stderr, "Unable to create variable kind 1, 2, 3 equivalence class (error code=%d, result=%p)\n", errcode, (void *) eqc);
		exit(EXIT_FAILURE);
	}
	if (count != 3) {
		fprintf(stderr, "Expected variable kind 1, 2, 3 equivalence class to have size 3, got %zu\n", count);
		exit(EXIT_FAILURE);
	}
	if (((vkind1 != eqc[0]) && (vkind1 != eqc[1]) && (vkind1 != eqc[2]))
			|| ((vkind2 != eqc[0]) && (vkind2 != eqc[1]) && (vkind2 != eqc[2]))
			|| ((vkind3 != eqc[0]) && (vkind3 != eqc[1]) && (vkind3 != eqc[2]))) {
		fprintf(stderr, "Expected equivalence class to contain variable kinds 1, 2, 3, got { %u, %u, %u }\n",
				(unsigned int) eqc[0], (unsigned int) eqc[1], (unsigned int) eqc[2]);
		exit(EXIT_FAILURE);
	}
	hilbert_array_free(eqc);
	hilbert_module_free(module);
}
