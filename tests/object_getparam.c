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
 * Test for hilbert_object_getparam()
 */

#include<stdio.h>
#include<stdlib.h>

#include"hilbert.h"

int main(void) {
	HilbertModule * src;
	HilbertModule * dest;
	HilbertHandle skind, dkind, param, handle;
	int errcode;

	src = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	dest = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	if ((src == NULL) || (dest == NULL)) {
		fputs("Unable to create interface modules\n", stderr);
		exit(EXIT_FAILURE);
	}
	skind = hilbert_kind_create(src, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create kind in source module (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_module_makeimmutable(src);
	if (errcode != 0) {
		fprintf(stderr, "Unable to make source module immutable (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	param = hilbert_module_param(dest, src, 0, NULL, NULL, NULL, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to parameterise dest with src (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	dkind = hilbert_object_getdesthandle(dest, param, skind, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain kind in destination module (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	handle = hilbert_object_getparam(dest, 666, &errcode);
	if (errcode != HILBERT_ERR_INVALID_HANDLE) {
		fprintf(stderr, "Expected invalid handle error, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	handle = hilbert_object_getparam(dest, param, &errcode);
	if (errcode != HILBERT_ERR_INVALID_HANDLE) {
		fprintf(stderr, "Expected wrong handle error, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	handle = hilbert_object_getparam(dest, dkind, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain kind parameter handle (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (handle != param) {
		fprintf(stderr, "Got wrong parameter handle (expected=%u, got=%u)\n", (unsigned int) param, (unsigned int) handle);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(src);
	hilbert_module_free(dest);
}
