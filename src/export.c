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

#include"private.h"

#include<stdlib.h>

#include"cl/pmap.h"
#include"cl/iset.h"
#include"cl/ivector.h"
#include"cl/ovector.h"

/**
 * Creates a parameter with empty handle map.
 *
 * @param src Pointer to source module.
 *
 * @return On success, a pointer to the newly created parameter is returned.
 * 	On error, <code>NULL</code> is returned.
 */
static union Object * param_create(struct HilbertModule * src) {
	union Object * result = malloc(sizeof(*result));
	if (result == NULL)
		goto noparammem;

	result->param = (struct Param) { .type = HILBERT_TYPE_PARAM, .module = src, .handle_map = hilbert_pmap_new() };
	if (result->param.handle_map == NULL)
		goto nomapmem;

	return result;

nomapmem:
	free(result);
noparammem:
	return NULL;
}

/**
 * Exports kinds of a source module from a destination module, checking the equivalence classes.
 *
 * @param dest Pointer to destination module, assumed to be locked.
 * @param src Pointer to source module, assumed to be locked.
 * @param argv Pointer to array of arguments to the parameters of the module pointed to by <code>src</code>.
 * 	If the number of elements in the array does not match the number of parameters, the behaviour is undefined.
 * @param mapper Pointer to parameter handle to argument handle mapper function.
 * @param userdata Pointer to user-defined data passed as an argument to the userdata parameter of <code>mapper</code>.
 * @param param Pointer to the new parameter.
 *
 * @return On success, <code>0</code> is returned. On error, a nonzero value is returned.
 */
static int export_kinds(struct HilbertModule * restrict dest, struct HilbertModule * restrict src,
		const HilbertHandle * restrict argv, HilbertMapperCallback mapper, void * userdata,
		struct Param * restrict param) {
	assert (dest != NULL);
	assert ((hilbert_ivector_count(src->paramhandles) == 0) || (argv != NULL));
	assert (mapper != NULL);
	assert (param != NULL);
	int errcode;

	/* Create index set for already handled kinds in equivalence check */
	IndexSet * already_handled = hilbert_iset_new();
	if (already_handled == NULL) {
		errcode = HILBERT_ERR_NOMEM;
		goto noahmem;
	}

	/* Inspect all source kinds */
	for (IndexVectorIterator i = hilbert_ivector_iterator_new(src->kindhandles);
			hilbert_ivector_iterator_hasnext(&i);) {
		HilbertHandle srckindhandle = hilbert_ivector_iterator_next(&i);
		union Object * srcobject = hilbert_ovector_get(src->objects, srckindhandle);
		assert (srcobject->generic.type & HILBERT_TYPE_KIND);
		HilbertHandle destkindhandle = mapper(dest, src, srckindhandle, userdata, &errcode);
		if (errcode != 0)
			goto error;
		union Object * destobject = hilbert_object_retrieve(dest, destkindhandle, HILBERT_TYPE_KIND);
		if (destobject == NULL) {
			errcode = HILBERT_ERR_INVALID_MAPPING;
			goto error;
		}
		if (srcobject->generic.type & HILBERT_TYPE_EXTERNAL) {
			/* check externality */
			if (!(destobject->generic.type & HILBERT_TYPE_EXTERNAL)) {
				errcode = HILBERT_ERR_INVALID_MAPPING;
				goto error;
			}
			struct ExternalKind * srckind = &srcobject->external_kind;
			HilbertHandle arghandle = argv[srckind->paramindex];
			struct ExternalKind * destkind = &destobject->external_kind;
			HilbertHandle destparamhandle = hilbert_ivector_get(dest->paramhandles, destkind->paramindex);
			if (destparamhandle != arghandle) {
				errcode = HILBERT_ERR_INVALID_MAPPING;
				goto error;
			}
		}
		const HilbertHandle * test = hilbert_pmap_post(param->handle_map, destkindhandle);
		if (test != NULL) {
			 errcode = HILBERT_ERR_MAPPING_CLASH;
			 goto error;
		}
		if (hilbert_pmap_add(param->handle_map, destkindhandle, srckindhandle) != 0) {
			errcode = HILBERT_ERR_NOMEM;
			goto error;
		}
	}

	/* check equivalence classes */
	for (IndexVectorIterator i = hilbert_ivector_iterator_new(src->kindhandles);
			hilbert_ivector_iterator_hasnext(&i);) {
		HilbertHandle srckindhandle = hilbert_ivector_iterator_next(&i);
		if (hilbert_iset_contains(already_handled, srckindhandle))
			continue;
		union Object * srcobject = hilbert_ovector_get(src->objects, srckindhandle);
		assert (srcobject != NULL);
		assert (srcobject->generic.type & HILBERT_TYPE_KIND);
		struct Kind * srckind = &srcobject->kind;
		if (srckind->equivalence_class == NULL)
			continue;
		const HilbertHandle * destkindhandle = hilbert_pmap_pre(param->handle_map, srckindhandle);
		assert (destkindhandle != NULL);
		for (IndexSetIterator j = hilbert_iset_iterator_new(srckind->equivalence_class);
				hilbert_iset_iterator_hasnext(&j);) {
			HilbertHandle srckindhandle2 = hilbert_iset_iterator_next(&j);
			const HilbertHandle * destkindhandle2 = hilbert_pmap_pre(param->handle_map, srckindhandle2);
			assert (destkindhandle2 != NULL);
			int rc = hilbert_kind_isequivalent(dest, *destkindhandle, *destkindhandle2, &errcode);
			assert (errcode == 0);
			if (!rc) {
				errcode = HILBERT_ERR_NO_EQUIVALENCE;
				goto error;
			}
			if (hilbert_iset_add(already_handled, srckindhandle2) != 0) {
				errcode = HILBERT_ERR_NOMEM;
				goto error;
			}
		}
	}

	errcode = 0;

error:
	hilbert_iset_del(already_handled);
noahmem:
	return errcode;
}

/**
 * Sets a dependency between two modules,
 * such that the destination module depends on the source module,
 * and the source module reverse-depends on the destination module.
 *
 * @param dest Pointer to the destination module, assumed to be locked.
 * @param src Pointer to the source module, assumed to be locked.
 *
 * @return On success, <code>0</code> is returned.
 * 	On error, a negative value is returned, which may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_NOMEM</code>:
 * 			There was not enough memory to perform the operation.
 */
static int set_dependency(struct HilbertModule * dest, struct HilbertModule * src) {
	assert (src != NULL);
	assert (dest != NULL);

	int contains_dep  = hilbert_mset_contains(dest->dependencies, src);
	int contains_rdep = hilbert_mset_contains(src->reverse_dependencies, dest);

	if ((!contains_dep) && (hilbert_mset_add(dest->dependencies, src) != 0))
		return HILBERT_ERR_NOMEM;

	if ((!contains_rdep) && (hilbert_mset_add(src->reverse_dependencies, dest) != 0)) {
		if (!contains_dep)
			hilbert_mset_remove(dest->dependencies, src);
		return HILBERT_ERR_NOMEM;
	}

	return 0;
}

HilbertHandle hilbert_module_export(struct HilbertModule * restrict dest, struct HilbertModule * restrict src,
		size_t argc, const HilbertHandle * restrict argv, HilbertMapperCallback mapper, void * userdata,
		int * restrict errcode) {
	assert (dest != NULL);
	assert (src != NULL);
	assert ((argc == 0) || (argv != NULL));
	assert (mapper != NULL);
	assert (errcode != NULL);

	HilbertHandle result = 0;
	int rc;

	/* locking & sanity checks */
	if (hilbert_module_gettype(dest) != HILBERT_PROOF_MODULE) {
		*errcode = HILBERT_ERR_INVALID_MODULE;
		goto invalidmodule;
	}

	if (hilbert_module_gettype(src) != HILBERT_INTERFACE_MODULE) {
		*errcode = HILBERT_ERR_INVALID_MODULE;
		goto invalidmodule;
	}

	if (mtx_lock(&dest->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto nodestlock;
	}

	if (mtx_lock(&src->mutex) != thrd_success) {
		*errcode = HILBERT_ERR_INTERNAL;
		goto nosrclock;
	}

	rc = hilbert_module_isimmutable(src, errcode);
	if ((*errcode == 0) && (!rc))
		*errcode = HILBERT_ERR_IMMUTABLE;
	if (*errcode != 0)
		goto immutable;

	if (hilbert_ivector_count(src->paramhandles) != argc) {
		*errcode = HILBERT_ERR_COUNT_MISMATCH;
		goto argerror;
	}

	for (size_t i = 0; i != argc; ++i) {
		if (hilbert_object_retrieve(dest, argv[i], HILBERT_TYPE_PARAM) == NULL) {
			*errcode = HILBERT_ERR_INVALID_HANDLE;
			goto argerror;
		}
	}

	/* parameter creation and export */
	union Object * param = param_create(src);
	if (param == NULL) {
		*errcode = HILBERT_ERR_NOMEM;
		goto noparammem;
	}

	size_t oldcount = hilbert_ovector_count(dest->objects);
	result = oldcount;

	if (hilbert_ovector_pushback(dest->objects, param) != 0) {
		*errcode = HILBERT_ERR_NOMEM;
		goto noobjectmem;
	}

	*errcode = export_kinds(dest, src, argv, mapper, userdata, &param->param); // FIXME
	if (*errcode != 0)
		goto kindexporterror;
	// FIXME: functors and statements
	
	if (hilbert_ivector_pushback(dest->paramhandles, result) != 0) {
		*errcode = HILBERT_ERR_NOMEM;
		goto noparamhandlemem;
	}

	*errcode = set_dependency(dest, src);
	if (*errcode != 0)
		goto deperror;

	goto success;

deperror:
noparamhandlemem:
kindexporterror:;
		size_t newcount = hilbert_ovector_count(dest->objects);
		for (size_t i = oldcount + 1; i < newcount; ++i) { /* param is freed below */
			hilbert_object_free(hilbert_ovector_get(dest->objects, i));
		}
		if (hilbert_ovector_downsize(dest->objects, oldcount) != 0)
			*errcode = HILBERT_ERR_INTERNAL;
noobjectmem:
	hilbert_param_free(param);
noparammem:
argerror:
immutable:
success:
	if (mtx_unlock(&src->mutex) != thrd_success)
		*errcode = HILBERT_ERR_INTERNAL;
nosrclock:
	if (mtx_unlock(&dest->mutex) != thrd_success)
		*errcode = HILBERT_ERR_INTERNAL;
nodestlock:
invalidmodule:
	return result;
}