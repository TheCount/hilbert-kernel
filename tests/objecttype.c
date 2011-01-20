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

#include<stdio.h>
#include<stdlib.h>

#include"hilbert.h"

int main(void) {
	HilbertModule * module;
	HilbertHandle object, object2;
	int errcode;
	unsigned int type;

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
	if (!(type & HILBERT_TYPE_KIND)) {
		fprintf(stderr, "Expected kind type, got 0x%4X\n", type);
		exit(EXIT_FAILURE);
	}

	object2 = hilbert_kind_alias(module, object, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to alias kind in interface module (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	type = hilbert_object_gettype(module, object2, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain alias kind type (error code=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (!(type & HILBERT_TYPE_KIND)) {
		fprintf(stderr, "Expected alias kind type, got 0x%4X\n", type);
		exit(EXIT_FAILURE);
	}

	// FIXME: test other object types, too

	hilbert_module_free(module);
}

