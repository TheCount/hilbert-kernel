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
 * Test to check equivalency restrictions of kinds vs. vkinds.
 */

#include<assert.h>
#include<stdio.h>
#include<stdlib.h>

#include"hilbert.h"

int main(void) {
	HilbertModule * src;
	HilbertModule * dest;
	HilbertHandle skind, svkind, dkind, dvkind;
	int errcode;

	src = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	assert (src != NULL);
	skind = hilbert_kind_create(src, &errcode);
	assert (errcode == 0);
	svkind = hilbert_vkind_create(src, &errcode);
	assert (errcode == 0);
	errcode = hilbert_kind_identify(src, skind, svkind);
	if (errcode != HILBERT_ERR_INVALID_HANDLE) {
		fprintf(stderr, "Expected invalid handle error upon identifying kind and variable kind in source module, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_module_makeimmutable(src);
	assert (errcode == 0);

	dest = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	assert (dest != NULL);
	HilbertHandle param = hilbert_module_param(dest, src, 0, NULL, NULL, NULL, &errcode);
	assert (errcode == 0);
	dkind = hilbert_object_getdesthandle(dest, param, skind, &errcode);
	assert (errcode == 0);
	dvkind = hilbert_object_getdesthandle(dest, param, svkind, &errcode);
	assert (errcode == 0);
	errcode = hilbert_kind_identify(dest, dkind, dvkind);
	if (errcode != HILBERT_ERR_INVALID_HANDLE) {
		fprintf(stderr, "Expected invalid handle error upon identifying kind and variable kind in destination module, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}

	hilbert_module_free(src);
	hilbert_module_free(dest);
}
