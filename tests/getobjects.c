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
 * Test for hilbert_module_getobjects()
 */

#include<stdio.h>
#include<stdlib.h>

#include"hilbert.h"

int main(void) {
	int errcode;
	size_t size;

	/* single module test */
	HilbertModule * module = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	if (module == NULL) {
		fputs("Unable to create Hilbert interface module\n", stderr);
		exit(EXIT_FAILURE);
	}
	HilbertHandle kind1 = hilbert_kind_create(module, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create kind1 (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	HilbertHandle kind2 = hilbert_kind_create(module, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create kind2 (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	// FIXME: other types of objects...
	HilbertHandle * objects = hilbert_module_getobjects(module, &size, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain module objects (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (size != 2) {
		fprintf(stderr, "Expected 2 objects in module, got %zu\n", size);
		exit(EXIT_FAILURE);
	}
	if ((objects[0] != kind1) || (objects[1] != kind2)) {
		fprintf(stderr, "Expected objects (%u, %u), got (%u, %u)\n", (unsigned int) kind1, (unsigned int) kind2,
				(unsigned int) objects[0], (unsigned int) objects[1]);
		exit(EXIT_FAILURE);
	}
	hilbert_array_free(objects);
	hilbert_module_free(module);

	/* two module test */
	HilbertModule * module1 = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	HilbertModule * module2 = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	if ((module1 == NULL) || (module2 == NULL)) {
		fputs("Unable to create Hilbert interface modules\n", stderr);
		exit(EXIT_FAILURE);
	}
	kind1 = hilbert_kind_create(module1, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create kind1 in module1 (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	kind2 = hilbert_kind_create(module2, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create kind2 in module2 (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_module_makeimmutable(module1);
	if (errcode != 0) {
		fprintf(stderr, "Unable to make module1 immutable (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	HilbertHandle param = hilbert_module_param(module2, module1, 0, NULL, NULL, NULL, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to parameterise module2 with module1 (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	HilbertHandle kind3 = hilbert_kind_create(module2, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create kind3 in module2 (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	objects = hilbert_module_getobjects(module2, &size, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain object array from module2 (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (size != 4) {
		fprintf(stderr, "Expected 4 objects in module2, got %zu\n", size);
		exit(EXIT_FAILURE);
	}
	if ((objects[0] != kind2) || (objects[3] != kind3)) {
		fprintf(stderr, "Expected (object0, object3) to be (kind2, kind3), got (%u, %u)\n",
				(unsigned int) objects[0], (unsigned int) objects[3]);
		exit(EXIT_FAILURE);
	}
	HilbertHandle destkind;
	if (objects[1] == param) {
		destkind = objects[2];
	} else if (objects[2] == param) {
		destkind = objects[1];
	} else {
		fprintf(stderr, "Parameter %u not found in {%u, %u}\n", (unsigned int) param,
				(unsigned int) objects[1], (unsigned int) objects[2]);
		exit(EXIT_FAILURE);
	}
	HilbertHandle srckind = hilbert_object_getsourcehandle(module2, destkind, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain source kind handle for kind1 (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (srckind != kind1) {
		fprintf(stderr, "Got wrong source kind (expected=%u, got=%u)\n",
				(unsigned int) kind1, (unsigned int) srckind);
		exit(EXIT_FAILURE);
	}
	hilbert_array_free(objects);
	hilbert_module_free(module1);
	hilbert_module_free(module2);
}
