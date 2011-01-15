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
 * This test attempts to create modules of several valid or invalid types.
 */

#include<stdio.h>
#include<stdlib.h>

#include"hilbert.h"

/**
 * Checks module creation for the specified type.
 *
 * @param type Module type.
 * @param expectfailure Whether to expect module creation to fail or not.
 */
static void check_module(int type, int expectfailure) {
	HilbertModule * module;

	module = hilbert_module_create(type);
	if (expectfailure && (module != NULL)) {
		fprintf(stderr, "Expected module creation to fail with invalid type %d\n", type);
		exit(EXIT_FAILURE);
	}
	if (!expectfailure && (module == NULL)) {
		fprintf(stderr, "Unable to create module with type %d\n", type);
		exit(EXIT_FAILURE);
	}
	// FIXME: Check module type with hilbert_module_gettype
	if (module != NULL)
		hilbert_module_free(module);
}

int main(void) {
	check_module(HILBERT_INTERFACE_MODULE, 0);
	check_module(HILBERT_PROOF_MODULE, 0);
	check_module(666, 1);
}
