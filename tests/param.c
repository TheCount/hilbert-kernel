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
 * Test to check interface parameterisation.
 */

#include<assert.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"hilbert.h"

/**
 * setting:
 * src2: kind0, {kind1, kind2}, kind3, kind4
 * src: kind5, {kind6, kind7}, param with src2, id kind0 and kind6, create kind8, id kind5 and kind3,
 * 	id kind8 and kind1
 * dest: kind9, param with src2, param with src(src2)
 * Expected eqc: {kind0, kind6, kind7}, {kind1, kind2, kind8}, {kind3, kind5}, kind4, kind9
 */
#define N_S2KINDS  5
#define N_SKINDS   9
#define N_DKINDS  10
static HilbertHandle s2kinds[N_S2KINDS];
static HilbertHandle skinds[N_SKINDS];
static HilbertHandle dkinds[N_DKINDS];
static HilbertHandle sf0, df0, s2f3, sf3, df3; /* functor handles */
static HilbertModule * dest;

/**
 * User data for testing.
 */
static int USERDATATEST = 12345;

/**
 * User error codes.
 */
#define USER_ERROR  1

/* identity callback */
static HilbertHandle callback_id(HilbertModule * restrict dest, HilbertModule * restrict src, HilbertHandle srcObject, void * userdata, int * restrict errcode) {
	if (dest == NULL) {
		fputs("Destination module in callback is NULL\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (src == NULL) {
		fputs("Source module in callback is NULL\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (*(int *) userdata != USERDATATEST) {
		fprintf(stderr, "Received invalid user data %p\n", userdata);
		exit(EXIT_FAILURE);
	}
	if (errcode == NULL) {
		fputs("Errcode ptr in callback is NULL\n", stderr);
		exit(EXIT_FAILURE);
	}
	*errcode = 0;
	for (size_t i = 0; i != N_SKINDS; ++i) {
		if (skinds[i] == srcObject)
			return dkinds[i];
	}
	if (sf0 == srcObject)
		return df0;
	if (sf3 == srcObject)
		return df3;
	fprintf(stderr, "Got invalid source object %u\n", (unsigned int) srcObject);
	exit(EXIT_FAILURE);
}

/* invalid handle callback */
static HilbertHandle callback_invalid_handle(HilbertModule * restrict dest, HilbertModule * restrict src, HilbertHandle srcObject, void * userdata, int * restrict errcode) {
	if (dest == NULL) {
		fputs("Destination module in callback is NULL\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (src == NULL) {
		fputs("Source module in callback is NULL\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (errcode == NULL) {
		fputs("Errcode ptr in callback is NULL\n", stderr);
		exit(EXIT_FAILURE);
	}
	*errcode = 0;
	return 666;
}

/* clashing callback */
static HilbertHandle callback_clash(HilbertModule * restrict dest, HilbertModule * restrict src, HilbertHandle srcObject, void * userdata, int * restrict errcode) {
	if (dest == NULL) {
		fputs("Destination module in callback is NULL\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (src == NULL) {
		fputs("Source module in callback is NULL\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (errcode == NULL) {
		fputs("Errcode ptr in callback is NULL\n", stderr);
		exit(EXIT_FAILURE);
	}
	*errcode = 0;
	return dkinds[0];
}

/* user error callback */
static HilbertHandle callback_error(HilbertModule * restrict dest, HilbertModule * restrict src, HilbertHandle srcObject, void * userdata, int * restrict errcode) {
	if (dest == NULL) {
		fputs("Destination module in callback is NULL\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (src == NULL) {
		fputs("Source module in callback is NULL\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (errcode == NULL) {
		fputs("Errcode ptr in callback is NULL\n", stderr);
		exit(EXIT_FAILURE);
	}
	*errcode = USER_ERROR;
	return 0;
}

/* dkind eqc checker */
static void eqc_check(size_t i1, size_t i2, int expected) {
	int errcode, rc;

	rc = hilbert_kind_isequivalent(dest, dkinds[i1], dkinds[i2], &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Failed to check equivalence of kind%zu and kind%zu in dest (errcode=%d)\n", i1, i2, errcode);
		exit(EXIT_FAILURE);
	}
	if (rc != expected) {
		fprintf(stderr, "Expected kind%zu and kind%zu in dest to be %sequivalent\n", i1, i2, expected ? "" : "in");
		exit(EXIT_FAILURE);
	}
}

int main(void) {
	HilbertModule * src;
	HilbertHandle param;
	int errcode, rc;
	size_t count;

	/* test with wrong kinds of modules */
	src = hilbert_module_create(HILBERT_PROOF_MODULE);
	dest = hilbert_module_create(HILBERT_PROOF_MODULE);
	if ((src == NULL) || (dest == NULL)) {
		fputs("Unable to create modules (src=proof, dest=proof)\n", stderr);
		exit(EXIT_FAILURE);
	}
	param = hilbert_module_param(dest, src, 0, NULL, NULL, NULL, &errcode);
	if (errcode != HILBERT_ERR_INVALID_MODULE) {
		fprintf(stderr, "Expected invalid module error (src=proof, dest=proof), got %d\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(dest);
	dest = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	if (dest == NULL) {
		fputs("Unable to create module (src=proof, dest=interface)\n", stderr);
		exit(EXIT_FAILURE);
	}
	param = hilbert_module_param(dest, src, 0, NULL, NULL, NULL, &errcode);
	if (errcode != HILBERT_ERR_INVALID_MODULE) {
		fprintf(stderr, "Expected invalid module error (src=proof, dest=interface), got %d\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(src);
	hilbert_module_free(dest);
	src = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	dest = hilbert_module_create(HILBERT_PROOF_MODULE);
	if ((src == NULL) || (dest == NULL)) {
		fputs("Unable to create modules (src=interface, dest=proof)\n", stderr);
		exit(EXIT_FAILURE);
	}
	param = hilbert_module_param(dest, src, 0, NULL, NULL, NULL, &errcode);
	if (errcode != HILBERT_ERR_INVALID_MODULE) {
		fprintf(stderr, "Expected invalid module error (src=interface, dest=proof), got %d\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(src);
	hilbert_module_free(dest);

	/* test with wrong kind of mutability */
	src = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	dest = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	if ((src == NULL) || (dest == NULL)) {
		fputs("Unable to create modules (src=interface, dest=interface)\n", stderr);
		exit(EXIT_FAILURE);
	}
	param = hilbert_module_param(dest, src, 0, NULL, NULL, NULL, &errcode);
	if (errcode != HILBERT_ERR_IMMUTABLE) {
		fprintf(stderr, "Expected mutability error (src=mutable, dest=mutable), got %d\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_module_makeimmutable(dest);
	if (errcode != 0) {
		fprintf(stderr, "Unable to make destination module immutable (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	param = hilbert_module_param(dest, src, 0, NULL, NULL, NULL, &errcode);
	if (errcode != HILBERT_ERR_IMMUTABLE) {
		fprintf(stderr, "Expected mutability error (src=mutable, dest=immutable), got %d\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_module_makeimmutable(src);
	if (errcode != 0) {
		fprintf(stderr, "Unable to make source module immutable (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	param = hilbert_module_param(dest, src, 0, NULL, NULL, NULL, &errcode);
	if (errcode != HILBERT_ERR_IMMUTABLE) {
		fprintf(stderr, "Expected mutability error (src=immutable, dest=immutable), got %d\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(src);
	hilbert_module_free(dest);

	/* test empty param */
	src = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	dest = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	if ((src == NULL) || (dest == NULL)) {
		fputs("Unable to create modules (src=interface, dest=interface)\n", stderr);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_module_makeimmutable(src);
	if (errcode != 0) {
		fprintf(stderr, "Unable to make source module immutable (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	param = hilbert_module_param(dest, src, 0, NULL, NULL, NULL, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to parameterise module (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(src);
	hilbert_module_free(dest);

	/* three-module test */
	src = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	HilbertModule * src2 = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	dest = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	if ((src == NULL) || (src2 == NULL) || (dest == NULL)) {
		fputs("Unable to create interface modules for three-module-test\n", stderr);
		exit(EXIT_FAILURE);
	}
	s2kinds[0] = hilbert_vkind_create(src2, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create variable kind0 in src2 (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	s2kinds[1] = hilbert_kind_create(src2, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create kind1 in src2 (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	s2kinds[2] = hilbert_kind_create(src2, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create kind2 in src2 (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_kind_identify(src2, s2kinds[1], s2kinds[2]);
	if (errcode != 0) {
		fprintf(stderr, "Unable to identify kind1 with kind2 in src2 (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	s2kinds[3] = hilbert_kind_create(src2, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create kind3 in src2 (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	s2kinds[4] = hilbert_kind_create(src2, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create kind4 in src2 (errcode=%d)\n", errcode);
		 exit(EXIT_FAILURE);
	}
	s2f3 = hilbert_functor_create(src2, s2kinds[1], 3, s2kinds, &errcode);
	assert (errcode == 0);
	// FIXME: other object types...
	errcode = hilbert_module_makeimmutable(src2);
	if (errcode != 0) {
		fprintf(stderr, "Unable to make Hilbert module src2 immutable (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	skinds[5] = hilbert_kind_create(src, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create kind5 in src (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	skinds[6] = hilbert_vkind_create(src, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create variable kind6 in src (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	skinds[7] = hilbert_vkind_create(src, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create variable kind7 in src (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_kind_identify(src, skinds[6], skinds[7]);
	if (errcode != 0) {
		fprintf(stderr, "Unable to identify kind6 with kind7 in src (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	sf0 = hilbert_functor_create(src, skinds[5], 0, NULL, &errcode);
	assert (errcode == 0);
	HilbertHandle param1 = hilbert_module_param(src, src2, 0, NULL, NULL, NULL, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to parameterise module src with module src2 (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	skinds[0] = hilbert_object_getdesthandle(src, param1, s2kinds[0], &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain kind0 in src (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	skinds[1] = hilbert_object_getdesthandle(src, param1, s2kinds[1], &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain kind1 in src (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	skinds[2] = hilbert_object_getdesthandle(src, param1, s2kinds[2], &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain kind2 in src (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	skinds[3] = hilbert_object_getdesthandle(src, param1, s2kinds[3], &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain kind3 in src (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	skinds[4] = hilbert_object_getdesthandle(src, param1, s2kinds[4], &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain kind4 in src (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	sf3 = hilbert_object_getdesthandle(src, param1, s2f3, &errcode);
	assert (errcode == 0);
	rc = hilbert_kind_isequivalent(src, skinds[1], skinds[2], &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to check equivalence of kind1 and kind2 in src (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (!rc) {
		fputs("Expected kind1 and kind2 to be equivalent in src\n", stderr);
		exit(EXIT_FAILURE);
	}
	rc = hilbert_kind_isequivalent(src, skinds[6], skinds[7], &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to check equivalence of kind6 and kind7 in src (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	if (!rc) {
		fputs("Expected kind6 and kind7 to be equivalent in src\n", stderr);
		exit(EXIT_FAILURE);
	}
	HilbertHandle * eqc = hilbert_kind_equivalenceclass(src, skinds[0], &count, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain equivalence class of kind0 in src (errcode=%d)\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (count != 1) {
		fprintf(stderr, "Wrong equivalence class size for kind0 in src (expected=1, got=%zu)\n", count);
		exit(EXIT_FAILURE);
	}
	hilbert_harray_free(eqc);
	eqc = hilbert_kind_equivalenceclass(src, skinds[1], &count, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain equivalence class of kind1 in src (errcode=%d)\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (count != 2) {
		fprintf(stderr, "Wrong equivalence class size for kind1 in src (expected=2, got=%zu)\n", count);
		exit(EXIT_FAILURE);
	}
	hilbert_harray_free(eqc);
	eqc = hilbert_kind_equivalenceclass(src, skinds[3], &count, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain equivalence class of kind3 in src (errcode=%d)\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (count != 1) {
		fprintf(stderr, "Wrong equivalence class size for kind3 in src (expected=1, got=%zu)\n", count);
		exit(EXIT_FAILURE);
	}
	hilbert_harray_free(eqc);
	eqc = hilbert_kind_equivalenceclass(src, skinds[4], &count, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain equivalence class of kind4 in src (errcode=%d)\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (count != 1) {
		fprintf(stderr, "Wrong equivalence class size for kind4 in src (expected=1, got=%zu)\n", count);
		exit(EXIT_FAILURE);
	}
	hilbert_harray_free(eqc);
	eqc = hilbert_kind_equivalenceclass(src, skinds[5], &count, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain equivalence class of kind5 in src (errcode=%d)\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (count != 1) {
		fprintf(stderr, "Wrong equivalence class size for kind5 in src (expected=1, got=%zu)\n", count);
		exit(EXIT_FAILURE);
	}
	hilbert_harray_free(eqc);
	eqc = hilbert_kind_equivalenceclass(src, skinds[6], &count, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain equivalence class of kind6 in src (errcode=%d)\n", stderr);
		exit(EXIT_FAILURE);
	}
	if (count != 2) {
		fprintf(stderr, "Wrong equivalence class size for kind6 in src (expected=2, got=%zu)\n", count);
		exit(EXIT_FAILURE);
	}
	hilbert_harray_free(eqc);
	errcode = hilbert_kind_identify(src, skinds[0], skinds[6]);
	if (errcode != 0) {
		fprintf(stderr, "Unable to identify kind0 with kind6 in src (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	skinds[8] = hilbert_kind_create(src, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create kind8 in src (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_kind_identify(src, skinds[5], skinds[3]);
	if (errcode != 0) {
		fprintf(stderr, "Unable to identify kind5 with kind3 in src (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_kind_identify(src, skinds[8], skinds[1]);
	if (errcode != 0) {
		fprintf(stderr, "Unable to identify kind8 with kind1 in src (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	errcode = hilbert_module_makeimmutable(src);
	if (errcode != 0) {
		fprintf(stderr, "Unable to make Hilbert module src immutable (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	dkinds[9] = hilbert_kind_create(dest, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to create kind9 in dest (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	HilbertHandle s2param = hilbert_module_param(dest, src2, 0, NULL, NULL, NULL, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to parameterise Hilbert module dest with src2 (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	dkinds[0] = hilbert_object_getdesthandle(dest, s2param, s2kinds[0], &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain kind0 in dest (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	dkinds[1] = hilbert_object_getdesthandle(dest, s2param, s2kinds[1], &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain kind1 in dest (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	dkinds[2] = hilbert_object_getdesthandle(dest, s2param, s2kinds[2], &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain kind2 in dest (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	dkinds[3] = hilbert_object_getdesthandle(dest, s2param, s2kinds[3], &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain kind3 in dest (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	dkinds[4] = hilbert_object_getdesthandle(dest, s2param, s2kinds[4], &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain kind4 in dest (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	df3 = hilbert_object_getdesthandle(dest, s2param, s2f3, &errcode);
	assert (errcode == 0);
	HilbertHandle sparam = hilbert_module_param(dest, src, 0, NULL, NULL, NULL, &errcode);
	if (errcode != HILBERT_ERR_COUNT_MISMATCH) {
		fprintf(stderr, "Expected count mismatch error, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	HilbertHandle argv[1] = { 666 };
	sparam = hilbert_module_param(dest, src, 1, argv, callback_id, &USERDATATEST, &errcode);
	if (errcode != HILBERT_ERR_INVALID_HANDLE) {
		fprintf(stderr, "Expected invalid handle error, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	argv[0] = s2param;
	sparam = hilbert_module_param(dest, src, 1, argv, callback_invalid_handle, NULL, &errcode);
	if (errcode != HILBERT_ERR_INVALID_MAPPING) {
		fprintf(stderr, "Expected invalid mapping error, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	sparam = hilbert_module_param(dest, src, 1, argv, callback_clash, NULL, &errcode);
	if (errcode != HILBERT_ERR_MAPPING_CLASH) {
		fprintf(stderr, "Expected mapping clash error, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	sparam = hilbert_module_param(dest, src, 1, argv, callback_error, NULL, &errcode);
	if (errcode != USER_ERROR) {
		fprintf(stderr, "Expected user generated error, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	sparam = hilbert_module_param(dest, src, 1, argv, callback_id, &USERDATATEST, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to parameterise Hilbert module dest with src (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	dkinds[5] = hilbert_object_getdesthandle(dest, sparam, skinds[5], &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain kind5 in dest (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	dkinds[6] = hilbert_object_getdesthandle(dest, sparam, skinds[6], &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain kind6 in dest (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	dkinds[7] = hilbert_object_getdesthandle(dest, sparam, skinds[7], &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain kind7 in dest (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	dkinds[8] = hilbert_object_getdesthandle(dest, sparam, skinds[8], &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to obtain kind8 in dest (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	df0 = hilbert_object_getdesthandle(dest, sparam, sf0, &errcode);
	assert (errcode == 0);
	eqc_check(0, 6, 1);
	eqc_check(0, 7, 1);
	eqc_check(0, 1, 0);
	eqc_check(0, 3, 0);
	eqc_check(0, 4, 0);
	eqc_check(0, 9, 0);
	eqc_check(1, 2, 1);
	eqc_check(1, 8, 1);
	eqc_check(1, 3, 0);
	eqc_check(1, 4, 0);
	eqc_check(1, 9, 0);
	eqc_check(3, 5, 1);
	eqc_check(3, 4, 0);
	eqc_check(3, 9, 0);
	eqc_check(4, 9, 0);
	HilbertHandle rkind = hilbert_functor_getkind(dest, df0, &errcode);
	assert (errcode == 0);
	if (rkind != dkinds[5]) {
		fprintf(stderr, "Constant functor has wrong result kind (expected: %u, got: %u)\n", (unsigned int) dkinds[5], (unsigned int) rkind);
		exit(EXIT_FAILURE);
	}
	HilbertHandle * ikinds = hilbert_functor_getinputkinds(dest, df0, &count, &errcode);
	assert (errcode == 0);
	if (count != 0) {
		fprintf(stderr, "Constant functor nonzero input kind count (count=%zu)\n", count);
		exit(EXIT_FAILURE);
	}
	hilbert_harray_free(ikinds);
	rkind = hilbert_functor_getkind(dest, df3, &errcode);
	assert (errcode == 0);
	if (rkind != dkinds[1]) {
		fprintf(stderr, "Functor has wrong result kind (expected: %u, got: %u)\n", (unsigned int) dkinds[1], (unsigned int) rkind);
		exit(EXIT_FAILURE);
	}
	ikinds = hilbert_functor_getinputkinds(dest, df3, &count, &errcode);
	assert (errcode == 0);
	if (count != 3) {
		fprintf(stderr, "Functor has wrong number of input kinds (expected: 3, got: %zu)\n", count);
		exit(EXIT_FAILURE);
	}
	if (memcmp(ikinds, dkinds, count * sizeof(*ikinds)) != 0) {
		fputs("Functor has wrong input kinds\n", stderr);
		exit(EXIT_FAILURE);
	}
	hilbert_harray_free(ikinds);
	hilbert_module_free(src);
	hilbert_module_free(src2);
	hilbert_module_free(dest);
}

