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
 * Test to check exports.
 */

#include<assert.h>
#include<stdio.h>
#include<stdlib.h>

#include"hilbert.h"

/**
 * User data for testing.
 */
static int USERDATATEST = 32154;

/**
 * User error codes.
 */
#define USER_ERROR 1

/* kind arrays for extended test.
 * src1: {0, 1}, 2
 * src2: 3, param src1, {4, 5}, 6, id 6 with 0.
 */
#define N_S1KINDS 3
#define N_S2KINDS 7
#define N_DKINDS  7
HilbertHandle s1kinds[N_S1KINDS];
HilbertHandle s2kinds[N_S2KINDS];
HilbertHandle dkinds[N_DKINDS];
HilbertHandle s2f0, s1f0, df0, s2f2, df2; /* functor handles */

/* identity callback */
static HilbertHandle callback_id(HilbertModule * restrict dest, HilbertModule * restrict src, HilbertHandle srcObject, void * userdata, int * restrict errcode) {
	assert ((dest != NULL) && (src != NULL) && (errcode != NULL));
	if (*(int *) userdata != USERDATATEST) {
		fprintf(stderr, "Received invalid user data %p\n", userdata);
		exit(EXIT_FAILURE);
	}
	*errcode = 0;
	for (size_t i = 0; i != N_S2KINDS; ++i) {
		if (s2kinds[i] == srcObject)
			return dkinds[i];
	}
	if (s2f0 == srcObject)
		return df0;
	if (s2f2 == srcObject)
		return df2;
	fprintf(stderr, "Got invalid source object %u\n", (unsigned int) srcObject);
	exit(EXIT_FAILURE);
}

/* invalid handle callback */
static HilbertHandle callback_invalid_handle(HilbertModule * restrict dest, HilbertModule * restrict src, HilbertHandle srcObject, void * userdata, int * restrict errcode) {
	assert ((dest != NULL) && (src != NULL) && (errcode != NULL));
	*errcode = 0;
	return 666;
}

/* clashing callback */
static HilbertHandle callback_clash(HilbertModule * restrict dest, HilbertModule * restrict src, HilbertHandle srcObject, void * userdata, int * restrict errcode) {
	assert ((dest != NULL) && (src != NULL) && (errcode != NULL));
	*errcode = 0;
	if ((srcObject == s2kinds[0]) || (srcObject == s2kinds[1]) || (srcObject == s2kinds[6])) { // careful to avoid clash preemption by invalid (v)kind mapping
		return dkinds[0];
	} else {
		return dkinds[2];
	}
}

/* user error callback */
static HilbertHandle callback_error(HilbertModule * restrict dest, HilbertModule * restrict src, HilbertHandle srcObject, void * userdata, int * restrict errcode) {
	assert ((dest != NULL) && (src != NULL) && (errcode != NULL));
	*errcode = USER_ERROR;
	return 0;
}

int main(void) {
	int errcode;

	/* immutability test */
	HilbertModule * src = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	HilbertModule * dest = hilbert_module_create(HILBERT_PROOF_MODULE);
	assert ((src != NULL) && (dest != NULL));
	HilbertHandle param = hilbert_module_export(dest, src, 0, NULL, callback_error, &USERDATATEST, &errcode);
	if (errcode != HILBERT_ERR_IMMUTABLE) {
		fprintf(stderr, "Expected immutability error, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(src);
	hilbert_module_free(dest);

	/* module type test */
	src = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	dest = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	assert ((src != NULL) && (dest != NULL));
	errcode = hilbert_module_makeimmutable(src);
	assert (errcode == 0);
	param = hilbert_module_export(dest, src, 0, NULL, callback_error, &USERDATATEST, &errcode);
	if (errcode != HILBERT_ERR_INVALID_MODULE) {
		fprintf(stderr, "Expected invalid module error, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(src);
	src = hilbert_module_create(HILBERT_PROOF_MODULE);
	assert (src != NULL);
	param = hilbert_module_export(dest, src, 0, NULL, callback_error, &USERDATATEST, &errcode);
	if (errcode != HILBERT_ERR_INVALID_MODULE) {
		fprintf(stderr, "Expected invalid module error, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(dest);
	dest = hilbert_module_create(HILBERT_PROOF_MODULE);
	assert (dest != NULL);
	param = hilbert_module_export(dest, src, 0, NULL, callback_error, &USERDATATEST, &errcode);
	if (errcode != HILBERT_ERR_INVALID_MODULE) {
		fprintf(stderr, "Expected invalid module error, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(src);
	src = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	assert (src != NULL);
	errcode = hilbert_module_makeimmutable(src);
	assert (errcode == 0);
	param = hilbert_module_export(dest, src, 0, NULL, callback_error, &USERDATATEST, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to export src from dest (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(src);
	hilbert_module_free(dest);

	/* count and invalid handle test */
	HilbertModule * src1 = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	HilbertModule * src2 = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	dest = hilbert_module_create(HILBERT_PROOF_MODULE);
	assert ((src1 != NULL) && (src2 != NULL) && (dest != NULL));
	errcode = hilbert_module_makeimmutable(src1);
	assert (errcode == 0);
	HilbertHandle param2 = hilbert_module_param(src2, src1, 0, NULL, NULL, NULL, &errcode);
	assert (errcode == 0);
	HilbertHandle src1param = hilbert_module_import(dest, src1, 0, NULL, NULL, NULL, &errcode);
	assert (errcode == 0);
	errcode = hilbert_module_makeimmutable(src2);
	assert (errcode == 0);
	HilbertHandle src2param = hilbert_module_export(dest, src2, 0, NULL, callback_error, &USERDATATEST, &errcode);
	if (errcode != HILBERT_ERR_COUNT_MISMATCH) {
		fprintf(stderr, "Expected count mismatch error, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	HilbertHandle invalid_argv[1] = { 666 };
	src2param = hilbert_module_export(dest, src2, 1, invalid_argv, callback_error, &USERDATATEST, &errcode);
	if (errcode != HILBERT_ERR_INVALID_HANDLE) {
		fprintf(stderr, "Expected invalid handle error, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(src2);
	hilbert_module_free(src1);
	hilbert_module_free(dest);

	/* a more extended test */
	src1 = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	src2 = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	assert ((src1 != NULL) && (src2 != NULL));
	s1kinds[0] = hilbert_vkind_create(src1, &errcode);
	assert (errcode == 0);
	s1kinds[1] = hilbert_vkind_create(src1, &errcode);
	assert (errcode == 0);
	errcode = hilbert_kind_identify(src1, s1kinds[0], s1kinds[1]);
	assert (errcode == 0);
	s1kinds[2] = hilbert_kind_create(src1, &errcode);
	assert (errcode == 0);
	s1f0 = hilbert_functor_create(src1, s1kinds[2], 0, NULL, &errcode);
	assert (errcode == 0);
	errcode = hilbert_module_makeimmutable(src1);
	assert (errcode == 0);
	s2kinds[3] = hilbert_kind_create(src2, &errcode);
	assert (errcode == 0);
	param2 = hilbert_module_param(src2, src1, 0, NULL, NULL, NULL, &errcode);
	assert (errcode == 0);
	s2kinds[0] = hilbert_object_getdesthandle(src2, param2, s1kinds[0], &errcode);
	assert (errcode == 0);
	s2kinds[1] = hilbert_object_getdesthandle(src2, param2, s1kinds[1], &errcode);
	assert (errcode == 0);
	s2kinds[2] = hilbert_object_getdesthandle(src2, param2, s1kinds[2], &errcode);
	assert (errcode == 0);
	s2kinds[4] = hilbert_kind_create(src2, &errcode);
	assert (errcode == 0);
	s2kinds[5] = hilbert_kind_create(src2, &errcode);
	assert (errcode == 0);
	errcode = hilbert_kind_identify(src2, s2kinds[4], s2kinds[5]);
	assert (errcode == 0);
	s2kinds[6] = hilbert_vkind_create(src2, &errcode);
	errcode = hilbert_kind_identify(src2, s2kinds[0], s2kinds[6]);
	assert (errcode == 0);
	s2f2 = hilbert_functor_create(src2, s2kinds[4], 2, &s2kinds[3], &errcode);
	assert (errcode == 0);
	s2f0 = hilbert_object_getdesthandle(src2, param2, s1f0, &errcode);
	assert (errcode == 0);
	errcode = hilbert_module_makeimmutable(src2);
	assert (errcode == 0);
	dest = hilbert_module_create(HILBERT_PROOF_MODULE);
	assert (dest != NULL);
	HilbertHandle s1param = hilbert_module_import(dest, src1, 0, NULL, NULL, NULL, &errcode);
	assert (errcode == 0);
	dkinds[0] = hilbert_object_getdesthandle(dest, s1param, s1kinds[0], &errcode);
	assert (errcode == 0);
	dkinds[1] = hilbert_object_getdesthandle(dest, s1param, s1kinds[1], &errcode);
	assert (errcode == 0);
	dkinds[2] = hilbert_object_getdesthandle(dest, s1param, s1kinds[2], &errcode);
	assert (errcode == 0);
	df0 = hilbert_object_getdesthandle(dest, s1param, s1f0, &errcode);
	assert (errcode == 0);
	HilbertModule * src3 = hilbert_module_create(HILBERT_INTERFACE_MODULE);
	assert (src3 != NULL);
	dkinds[3] = hilbert_kind_create(src3, &errcode);
	assert (errcode == 0);
	dkinds[4] = hilbert_kind_create(src3, &errcode);
	assert (errcode == 0);
	HilbertHandle s3kindfake = hilbert_vkind_create(src3, &errcode);
	assert (errcode == 0);
	HilbertHandle df2real = hilbert_functor_create(src3, dkinds[4], 2, &dkinds[3], &errcode);
	assert (errcode == 0);
	HilbertHandle df2fake1 = hilbert_functor_create(src3, dkinds[3], 2, &dkinds[3], &errcode);
	assert (errcode == 0);
	HilbertHandle df2fake2 = hilbert_functor_create(src3, dkinds[4], 1, &dkinds[3], &errcode);
	assert (errcode == 0);
	HilbertHandle fake_ikinds[2] = { dkinds[4], dkinds[3] };
	HilbertHandle df2fake3 = hilbert_functor_create(src3, dkinds[4], 2, fake_ikinds, &errcode);
	assert (errcode == 0);
	errcode = hilbert_module_makeimmutable(src3);
	assert (errcode == 0);
	HilbertHandle s3param = hilbert_module_import(dest, src3, 0, NULL, NULL, NULL, &errcode);
	assert (errcode == 0);
	dkinds[3] = hilbert_object_getdesthandle(dest, s3param, dkinds[3], &errcode);
	assert (errcode == 0);
	dkinds[4] = hilbert_object_getdesthandle(dest, s3param, dkinds[4], &errcode);
	assert (errcode == 0);
	dkinds[5] = hilbert_kind_alias(dest, dkinds[4], &errcode);
	assert (errcode == 0);
	HilbertHandle dkinds6real = hilbert_kind_alias(dest, dkinds[0], &errcode);
	assert (errcode == 0);
	HilbertHandle dkinds6fake = hilbert_object_getdesthandle(dest, s3param, s3kindfake, &errcode);
	assert (errcode == 0);
	dkinds[6] = dkinds6real; 
	df2real = hilbert_object_getdesthandle(dest, s3param, df2real, &errcode);
	assert (errcode == 0);
	df2fake1 = hilbert_object_getdesthandle(dest, s3param, df2fake1, &errcode);
	assert (errcode == 0);
	df2fake2 = hilbert_object_getdesthandle(dest, s3param, df2fake2, &errcode);
	assert (errcode == 0);
	df2fake3 = hilbert_object_getdesthandle(dest, s3param, df2fake3, &errcode);
	assert (errcode == 0);
	df2 = df2real;
	HilbertHandle argv[1] = { s1param };
	HilbertHandle s2param = hilbert_module_export(dest, src2, 1, argv, callback_invalid_handle, &USERDATATEST, &errcode);
	if (errcode != HILBERT_ERR_INVALID_MAPPING) {
		fprintf(stderr, "Expected invalid mapping error, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	s2param = hilbert_module_export(dest, src2, 1, argv, callback_clash, &USERDATATEST, &errcode);
	if (errcode != HILBERT_ERR_MAPPING_CLASH) {
		fprintf(stderr, "Expected mapping clash, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	s2param = hilbert_module_export(dest, src2, 1, argv, callback_error, &USERDATATEST, &errcode);
	if (errcode != USER_ERROR) {
		fprintf(stderr, "Expected user error, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	dkinds[6] = dkinds6fake;
	s2param = hilbert_module_export(dest, src2, 1, argv, callback_id, &USERDATATEST, &errcode);
	if (errcode != HILBERT_ERR_NO_EQUIVALENCE) {
		fprintf(stderr, "Expected missing equivalence error, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	dkinds[6] = dkinds6real;
	df2 = df2fake1;
	s2param = hilbert_module_export(dest, src2, 1, argv, callback_id, &USERDATATEST, &errcode);
	if (errcode != HILBERT_ERR_INVALID_MAPPING) {
		fprintf(stderr, "Expected invalid mapping error for wrong result kind, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	df2 = df2fake2;
	s2param = hilbert_module_export(dest, src2, 1, argv, callback_id, &USERDATATEST, &errcode);
	if (errcode != HILBERT_ERR_INVALID_MAPPING) {
		fprintf(stderr, "Expected invalid mapping error for wrong place count, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	df2 = df2fake3;
	s2param = hilbert_module_export(dest, src2, 1, argv, callback_id, &USERDATATEST, &errcode);
	if (errcode != HILBERT_ERR_INVALID_MAPPING) {
		fprintf(stderr, "Expected invalid mapping error for wrong input kinds, got errcode=%d instead\n", errcode);
		exit(EXIT_FAILURE);
	}
	df2 = df2real;
	s2param = hilbert_module_export(dest, src2, 1, argv, callback_id, &USERDATATEST, &errcode);
	if (errcode != 0) {
		fprintf(stderr, "Unable to export src2 from dest (errcode=%d)\n", errcode);
		exit(EXIT_FAILURE);
	}
	hilbert_module_free(src1);
	hilbert_module_free(src2);
	hilbert_module_free(src3);
	hilbert_module_free(dest);

	// FIXME: other object types, externality tests...
}
