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
 * Test to check object types.
 */

#include<assert.h>
#include<stdio.h>
#include<stdlib.h>

#include"hilbert.h"

int main(void) {
	HilbertModule * module;
	HilbertHandle object, object2;
	int errcode;
	unsigned int type, type2;

	module = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	if (module == NULL) {
		fputs("Unable to create Hilbert module\n", stderr);
		exit(EXIT_FAILURE);
	}

	type = hilbert_object_gettype(module, 666, &errcode);
	if (errcode != HILBERT_ERR_INVALID_HANDLE) {
		fprintf(stderr, "Expected invalid handle error, got %d\n", errcode);
		exit(EXIT_FAILURE);
	}

	/* kinds */
	object = hilbert_kind_create(module, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create kind in interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	type = hilbert_object_gettype(module, object, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain kind type (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if ((!(type & HILBERT_TYPE_KIND)) || (type & HILBERT_TYPE_VKIND)) {
		fprintf(stderr, "Expected kind type, got 0x%4X\n", type);
		exit(EXIT_FAILURE);
	}

	object2 = hilbert_kind_alias(module, object, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to alias kind in interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	type2 = hilbert_object_gettype(module, object2, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain alias kind type (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if ((!(type2 & HILBERT_TYPE_KIND)) || (type2 & HILBERT_TYPE_VKIND)) {
		fprintf(stderr, "Expected alias kind type, got 0x%04X\n", type2);
		exit(EXIT_FAILURE);
	}

	/* variable kinds */
	object = hilbert_vkind_create(module, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create variable kind in interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	type = hilbert_object_gettype(module, object, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain variable kind type (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if ((!(type & HILBERT_TYPE_KIND)) || (!(type & HILBERT_TYPE_VKIND))) {
		fprintf(stderr, "Expected variable kind type, got 0x%04X\n", type);
		exit(EXIT_FAILURE);
	}

	object2 = hilbert_kind_alias(module, object, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to alias variable kind in interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	type2 = hilbert_object_gettype(module, object2, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain alias kind type (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if ((!(type2 & HILBERT_TYPE_KIND)) || (!(type2 & HILBERT_TYPE_VKIND))) {
		fprintf(stderr, "Expected alias kind type, got 0x%04X\n", type2);
		exit(EXIT_FAILURE);
	}

	/* variables */
	object = hilbert_var_create(module, object2, &errcode);
	assert (errcode == 0);
	type = hilbert_object_gettype(module, object, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain type of variable object (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (!(type & HILBERT_TYPE_VAR)) {
		fprintf(stderr, "Expected variable type, got 0x%04X\n", type);
		exit(EXIT_FAILURE);
	}

	// FIXME: test other object types, too

	hilbert_module_free(module);
}

