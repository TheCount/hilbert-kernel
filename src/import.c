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
#include"cl/eset.h"
#include"cl/iset.h"
#include"cl/mset.h"

#include"threads/hthreads.h"

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
 * Creates a backup of existing kind equivalence classes.
 *
 * @param module Pointer to Hilbert module in which the kinds reside.
 * @param handle_map Pointer to parameter map, the domain of which contains the kinds whose equivalence classes
 * 	are to be backed up.
 *
 * @return On success, a pointer to a set of equivalence classes, the backup, is returned.
 * 	On error, <code>NULL</code> is returned.
 *
 * @sa #eqc_backup_restore()
 * @sa #eqc_backup_free()
 */
static EQCSet * eqc_backup_create(struct HilbertModule * restrict module, ParamMap * restrict handle_map) {
	assert (module != NULL);
	assert (handle_map != NULL);

	EQCSet * result = NULL;

	IndexSet * already_handled = hilbert_iset_new(); /* already handled kind handles */
	if (already_handled == NULL)
		goto noisetmem;

	result = hilbert_eset_new();
	if (result == NULL)
		goto noeqcsetmem;

	for (ParamMapIterator i = hilbert_pmap_iterator_new(handle_map); hilbert_pmap_iterator_hasnext(&i);) {
		struct ParamMapEntry entry = hilbert_pmap_iterator_next(&i);
		if (hilbert_iset_contains(already_handled, entry.pre))
			continue;
		union Object * object = hilbert_object_retrieve(module, entry.pre, HILBERT_TYPE_KIND);
		assert (object != NULL);
		struct Kind * kind = &object->kind;
		if (kind->equivalence_class == NULL)
			continue;
		IndexSet * equivalence_class = hilbert_iset_clone(kind->equivalence_class);
		if (equivalence_class == NULL)
			goto noeqcmem;
		if (hilbert_eset_add(result, equivalence_class) != 0) {
			hilbert_iset_del(equivalence_class);
			goto noeqcmem;
		}
		if (hilbert_iset_addall(already_handled, equivalence_class) != 0)
			goto noeqcmem;
	}

	goto success;

noeqcmem:
	for (EQCSetIterator i = hilbert_eset_iterator_new(result); hilbert_eset_iterator_hasnext(&i);)
		hilbert_iset_del(hilbert_eset_iterator_next(&i));
	hilbert_eset_del(result);
	result = NULL;
success:
noeqcsetmem:
	hilbert_iset_del(already_handled);
noisetmem:
	return result;
}

/**
 * Restores existing kind equivalence classes from a backup.
 *
 * @param module Pointer to the Hilbert module in which the kind equivalence classes are to be restored.
 * @param handle_map Pointer to parameter map, the domain of which contains the handles of the kinds
 * 	whose equivalence classes are to be restored.
 * @param backup Pointer to a set of equivalence classes, the backup. After the backup has been restored,
 * 	the set will be freed.
 *
 * @sa #eqc_backup_create()
 * @sa #eqc_backup_free()
 */
static void eqc_backup_restore(struct HilbertModule * restrict module, ParamMap * restrict handle_map,
		EQCSet * restrict backup) {
	assert (module != NULL);
	assert (handle_map != NULL);
	assert (backup != NULL);

	/* reset current equivalence classes to NULL */
	for (ParamMapIterator i = hilbert_pmap_iterator_new(handle_map); hilbert_pmap_iterator_hasnext(&i);) {
		struct ParamMapEntry entry = hilbert_pmap_iterator_next(&i);
		union Object * object = hilbert_object_retrieve(module, entry.pre, HILBERT_TYPE_KIND);
		assert (object != NULL);
		IndexSet * equivalence_class = object->kind.equivalence_class;
		if (equivalence_class == NULL)
			continue;
		for (IndexSetIterator j = hilbert_iset_iterator_new(equivalence_class);
				hilbert_iset_iterator_hasnext(&j);) {
			HilbertHandle kindhandle = hilbert_iset_iterator_next(&j);
			union Object * object2 = hilbert_object_retrieve(module, kindhandle, HILBERT_TYPE_KIND);
			assert (object2 != NULL);
			object2->kind.equivalence_class = NULL;
		}
		hilbert_iset_del(equivalence_class);
	}

	/* restore backup */
	for (EQCSetIterator i = hilbert_eset_iterator_new(backup); hilbert_eset_iterator_hasnext(&i);) {
		IndexSet * equivalence_class = hilbert_eset_iterator_next(&i);
		assert (equivalence_class != NULL);
		for (IndexSetIterator j = hilbert_iset_iterator_new(equivalence_class);
				hilbert_iset_iterator_hasnext(&j);) {
			HilbertHandle kindhandle = hilbert_iset_iterator_next(&j);
			union Object * object = hilbert_object_retrieve(module, kindhandle, HILBERT_TYPE_KIND);
			assert (object != NULL);
			object->kind.equivalence_class = equivalence_class;
		}
	}

	/* delete backup set */
	hilbert_eset_del(backup);
}

/**
 * Frees a backup of existing kind equivalence classes which is no longer needed.
 *
 * @param backup Pointer to backup set.
 *
 * @sa #eqc_backup_create()
 * @sa #eqc_backup_restore()
 */
static void eqc_backup_free(EQCSet * backup) {
	assert (backup != NULL);

	for (EQCSetIterator i = hilbert_eset_iterator_new(backup); hilbert_eset_iterator_hasnext(&i);)
		hilbert_iset_del(hilbert_eset_iterator_next(&i));

	hilbert_eset_del(backup);
}

/**
 * Loads kinds from a source module into a destination module, creating proper equivalence classes.
 *
 * @param dest Pointer to destination module, assumed to be locked.
 * @param src Pointer to source module, assumed to be locked.
 * @param argv Pointer to array of arguments to the parameters of the module pointed to by <code>src</code>.
 * 	If the number of elements in the array does not match the number of parameters, the behaviour is undefined.
 * @param mapper Pointer to parameter handle to argument handle mapper function.
 * 	If the number of parameters is zero, <code>mapper</code> may be <code>NULL</code>.
 * @param param Pointer to the new parameter.
 * @param paramindex Index of the new parameter in <code>dest</code>.
 *
 * Warning: this function adds elements to <code>dest->objects</code> and <code>dest->kindhandles</code>
 * without deleting them on error. It is up to the caller to do that.
 *
 * @return On success, <code>0</code> is returned. On error, a nonzero value is returned.
 */
static int load_kinds(HilbertModule * restrict dest, HilbertModule * restrict src, const HilbertHandle * restrict argv,
		HilbertMapperCallback mapper, struct Param * param, size_t paramindex) {
	assert (dest != NULL);
	assert (src != NULL);
	assert ((hilbert_ivector_count(src->paramhandles) == 0) || (argv != NULL));
	assert ((hilbert_ivector_count(src->paramhandles) == 0) || (mapper != NULL));
	assert (param != NULL);
	int errcode;

	IndexSet * already_handled = hilbert_iset_new(); /* kind handles which have already been handled */
	if (already_handled == NULL) {
		errcode = HILBERT_ERR_NOMEM;
		goto noahsetmem;
	}

	/* inspect all source kinds */
	for (IndexVectorIterator i = hilbert_ivector_iterator_new(src->kindhandles);
			hilbert_ivector_iterator_hasnext(&i);) {
		HilbertHandle srckindhandle = hilbert_ivector_iterator_next(&i);
		union Object * srcobject = hilbert_ovector_get(src->objects, srckindhandle);
		assert (srcobject->generic.type & HILBERT_TYPE_KIND);
		if (srcobject->generic.type & HILBERT_TYPE_EXTERNAL) {
			/* map to existing kind */
			struct ExternalKind * srckind = &srcobject->external_kind;
			HilbertHandle arghandle = argv[srckind->paramindex];
			HilbertHandle destkindhandle = mapper(dest, src, srckindhandle, &errcode);
			if (errcode != 0)
				goto error;
			union Object * destobject = hilbert_object_retrieve(dest, destkindhandle, HILBERT_TYPE_KIND);
			if ((destobject == NULL) || (!(destobject->generic.type & HILBERT_TYPE_EXTERNAL))) {
				errcode = HILBERT_ERR_INVALID_MAPPING;
				goto error;
			}
			struct ExternalKind * destkind = &destobject->external_kind;
			size_t destparamhandle = hilbert_ivector_get(dest->paramhandles, destkind->paramindex);
			if (destparamhandle != arghandle) {
				errcode = HILBERT_ERR_INVALID_MAPPING;
				goto error;
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
		} else {
			/* map to new kind */
			HilbertHandle destkindhandle = hilbert_ovector_count(dest->objects);
			union Object * destkind = malloc(sizeof(*destkind));
			if (destkind == NULL) {
				errcode = HILBERT_ERR_NOMEM;
				goto error;
			}
			if (hilbert_ovector_pushback(dest->objects, destkind) != 0) {
				free(destkind);
				errcode = HILBERT_ERR_NOMEM;
				goto error;
			}
			if (hilbert_ivector_pushback(dest->kindhandles, destkindhandle) != 0) {
				errcode = HILBERT_ERR_NOMEM;
				goto error;
			}
			destkind->external_kind = (struct ExternalKind) {
				.type = HILBERT_TYPE_KIND | HILBERT_TYPE_EXTERNAL,
				.equivalence_class = NULL,
				.paramindex = paramindex
			};
			if (hilbert_pmap_add(param->handle_map, destkindhandle, srckindhandle) != 0) {
				errcode = HILBERT_ERR_NOMEM;
				goto error;
			}
		}
	}

	/* coarsen kind equivalence relation in dest to become compatible with src */
	EQCSet * backup = eqc_backup_create(dest, param->handle_map);
	if (backup == NULL) {
		errcode = HILBERT_ERR_NOMEM;
		goto nobackupmem;
	}
	for (ParamMapIterator i = hilbert_pmap_iterator_new(param->handle_map); hilbert_pmap_iterator_hasnext(&i);) {
		struct ParamMapEntry entry = hilbert_pmap_iterator_next(&i);
		if (hilbert_iset_contains(already_handled, entry.post))
			continue;
		union Object * srcobject = hilbert_object_retrieve(src, entry.post, HILBERT_TYPE_KIND);
		assert (srcobject != NULL);
		struct Kind * srckind = &srcobject->kind;
		if (srckind->equivalence_class != NULL) {
			for (IndexSetIterator j = hilbert_iset_iterator_new(srckind->equivalence_class); hilbert_iset_iterator_hasnext(&j);) {
				HilbertHandle srckindhandle = hilbert_iset_iterator_next(&j);
				const HilbertHandle * destkindhandle = hilbert_pmap_pre(param->handle_map, srckindhandle);
				assert (destkindhandle != NULL);
				errcode = hilbert_kind_identify_nocheck(dest, entry.pre, *destkindhandle);
				assert ((errcode != HILBERT_ERR_INVALID_MODULE)
						&& (errcode != HILBERT_ERR_IMMUTABLE)
						&& (errcode != HILBERT_ERR_INVALID_HANDLE));
				if (errcode != 0) {
					eqc_backup_restore(dest, param->handle_map, backup);
					goto iderror;
				}
			}
		}
	}

	errcode = 0;

	eqc_backup_free(backup);
iderror:
nobackupmem:
error:
	hilbert_iset_del(already_handled);
noahsetmem:
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

HilbertHandle hilbert_module_param(HilbertModule * restrict dest, HilbertModule * restrict src, size_t argc,
		const HilbertHandle * restrict argv, HilbertMapperCallback mapper, int * restrict errcode) {
	assert (dest != NULL);
	assert (src != NULL);
	assert ((argc == 0) || (argv != NULL));
	assert ((argc == 0) || (mapper != NULL));
	assert (errcode != NULL);

	int rc;
	HilbertHandle result = 0;

	/* locking & sanity checks */
	if (hilbert_module_gettype(dest) != HILBERT_INTERFACE_MODULE) {
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

	rc = hilbert_module_isimmutable(dest, errcode);
	if ((*errcode == 0) && (rc))
		*errcode = HILBERT_ERR_IMMUTABLE;
	if (*errcode != 0)
		goto immutable;

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

	/* parameter creation and loading */
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

	size_t paramindex = hilbert_ivector_count(dest->paramhandles);
	*errcode = load_kinds(dest, src, argv, mapper, &param->param, paramindex);
	if (*errcode != 0)
		goto kindloaderror;
	// FIXME: functors
	
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
kindloaderror:;
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

HilbertHandle hilbert_module_import(HilbertModule * restrict dest, HilbertModule * restrict src, size_t argc,
		const HilbertHandle * restrict argv, HilbertMapperCallback mapper, int * restrict errcode) {
	assert (dest != NULL);
	assert (src != NULL);
	assert ((argc == 0) || (argv != NULL));
	assert ((argc == 0) || (mapper != NULL));
	assert (errcode != NULL);

	int rc;
	HilbertHandle result = 0;

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

	/* parameter creation and loading */
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

	size_t paramindex = hilbert_ivector_count(dest->paramhandles);
	*errcode = load_kinds(dest, src, argv, mapper, &param->param, paramindex);
	if (*errcode != 0)
		goto kindloaderror;
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
kindloaderror:;
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
