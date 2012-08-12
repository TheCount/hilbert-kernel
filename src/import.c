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

#include"private.h"
#include"param.h"

#include<assert.h>
#include<stdlib.h>

#include"cl/pmap.h"
#include"cl/eset.h"
#include"cl/iset.h"
#include"cl/mset.h"

#include"threads/hthreads.h"

/**
 * Initialises a backup of existing kind equivalence classes.
 *
 * @param backup Pointer to backup set to be initialised.
 * @param module Pointer to Hilbert module in which the kinds reside.
 * @param handle_map Pointer to parameter map, the domain of which contains the kinds whose equivalence classes
 * 	are to be backed up.
 *
 * @return On success, 0 is returned.
 * 	On error, -1 is returned.
 *
 * @sa #eqc_backup_restore()
 * @sa #eqc_backup_fini()
 */
static int eqc_backup_init( EQCSet * restrict backup, struct HilbertModule * restrict module, ParamMap * restrict handle_map ) {
	assert( backup != NULL );
	assert (module != NULL);
	assert (handle_map != NULL);

	int result = -1;

	IndexSet already_handled;
	int rc = hilbert_iset_init( &already_handled ); /* already handled kind handles */
	if ( rc != 0 ) {
		goto noisetmem;
	}

	rc = hilbert_eset_init( backup );
	if ( rc != 0 ) {
		goto noeqcsetmem;
	}

	for ( void * i = hilbert_pmap_iterator_start( handle_map ); i != NULL; i = hilbert_pmap_iterator_next( handle_map, i ) ) {
		struct ParamMapEntry entry = hilbert_pmap_iterator_get( handle_map, i );
		if ( hilbert_iset_contains( &already_handled, entry.pre ) ) {
			continue;
		}
		union Object * object = hilbert_object_retrieve(module, entry.pre, HILBERT_TYPE_KIND);
		assert (object != NULL);
		struct Kind * kind = &object->kind;
		if (kind->equivalence_class == NULL)
			continue;
		IndexSet * equivalence_class = hilbert_iset_clone(kind->equivalence_class);
		if (equivalence_class == NULL)
			goto noeqcmem;
		if ( hilbert_eset_add( backup, equivalence_class ) != 0 ) {
			hilbert_iset_del(equivalence_class);
			goto noeqcmem;
		}
		if ( hilbert_iset_addall( &already_handled, equivalence_class ) != 0 ) {
			goto noeqcmem;
		}
	}

	result = 0;
	goto success;

noeqcmem:
	for ( void * i = hilbert_eset_iterator_start( backup ); i != NULL; i = hilbert_eset_iterator_next( backup, i ) ) {
		hilbert_iset_del( hilbert_eset_iterator_get( backup, i ) );
	}
	hilbert_eset_fini( backup );
success:
noeqcsetmem:
	hilbert_iset_fini( &already_handled );
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
	for ( void * i = hilbert_pmap_iterator_start( handle_map ); i != NULL; i = hilbert_pmap_iterator_next( handle_map, i ) ) {
		struct ParamMapEntry entry = hilbert_pmap_iterator_get( handle_map, i );
		union Object * object = hilbert_object_retrieve(module, entry.pre, HILBERT_TYPE_KIND);
		assert (object != NULL);
		IndexSet * equivalence_class = object->kind.equivalence_class;
		if (equivalence_class == NULL)
			continue;
		for ( void * j = hilbert_iset_iterator_start( equivalence_class ); j != NULL; j = hilbert_iset_iterator_next( equivalence_class, j ) ) {
			HilbertHandle kindhandle = hilbert_iset_iterator_get( equivalence_class, j );
			union Object * object2 = hilbert_object_retrieve(module, kindhandle, HILBERT_TYPE_KIND);
			assert (object2 != NULL);
			object2->kind.equivalence_class = NULL;
		}
		hilbert_iset_del(equivalence_class);
	}

	/* restore backup */
	for ( void * i = hilbert_eset_iterator_start( backup ); i != NULL; i = hilbert_eset_iterator_next( backup, i ) ) {
		IndexSet * equivalence_class = hilbert_eset_iterator_get( backup, i );
		assert (equivalence_class != NULL);
		for ( void * j = hilbert_iset_iterator_start( equivalence_class ); j != NULL; j = hilbert_iset_iterator_next( equivalence_class, j ) ) {
			HilbertHandle kindhandle = hilbert_iset_iterator_get( equivalence_class, j );
			union Object * object = hilbert_object_retrieve(module, kindhandle, HILBERT_TYPE_KIND);
			assert (object != NULL);
			object->kind.equivalence_class = equivalence_class;
		}
	}

	/* delete backup set */
	hilbert_eset_del(backup);
}

/**
 * Uninitialises a backup of existing kind equivalence classes which is no longer needed.
 *
 * @param backup Pointer to backup set.
 *
 * @sa #eqc_backup_create()
 * @sa #eqc_backup_restore()
 */
static void eqc_backup_fini( EQCSet * backup ) {
	assert (backup != NULL);

	for ( void * i = hilbert_eset_iterator_start( backup ); i != NULL; i = hilbert_eset_iterator_next( backup, i ) ) {
		hilbert_iset_del( hilbert_eset_iterator_get( backup, i ) );
	}

	hilbert_eset_fini( backup );
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
 * @param userdata Pointer to user-defined data passed as an argument to the userdata parameter of <code>mapper</code>.
 * @param param Pointer to the new parameter.
 * @param paramindex Index of the new parameter in <code>dest</code>.
 *
 * Warning: this function adds elements to <code>dest->objects</code> and <code>dest->kindhandles</code>
 * without deleting them on error. It is up to the caller to do that.
 *
 * @return On success, <code>0</code> is returned. On error, a nonzero value is returned.
 */
static int load_kinds(HilbertModule * restrict dest, HilbertModule * restrict src, const HilbertHandle * restrict argv,
		HilbertMapperCallback mapper, void * userdata, struct Param * param, size_t paramindex) {
	assert (dest != NULL);
	assert (src != NULL);
	assert ( ( hilbert_ivector_count( &src->paramhandles ) == 0 ) || ( argv != NULL ) );
	assert ( ( hilbert_ivector_count( &src->paramhandles ) == 0 ) || ( mapper != NULL ) );
	assert (param != NULL);
	int errcode;

	IndexSet already_handled;
	errcode = hilbert_iset_init( &already_handled ); /* kind handles which have already been handled */
	if ( errcode != 0 ) {
		errcode = HILBERT_ERR_NOMEM;
		goto noahsetmem;
	}

	/* inspect all source kinds */
	for ( void * i = hilbert_ivector_iterator_start( &src->kindhandles ); i != NULL; i = hilbert_ivector_iterator_next( &src->kindhandles, i ) ) {
		HilbertHandle srckindhandle = hilbert_ivector_iterator_get( &src->kindhandles, i );
		union Object * srcobject = hilbert_ovector_get( &src->objects, srckindhandle );
		assert (srcobject->generic.type & HILBERT_TYPE_KIND);
		if (srcobject->generic.type & HILBERT_TYPE_EXTERNAL) {
			/* map to existing kind */
			struct ExternalKind * srckind = &srcobject->external_kind;
			HilbertHandle arghandle = argv[srckind->paramindex];
			HilbertHandle destkindhandle = mapper(dest, src, srckindhandle, userdata, &errcode);
			if (errcode != 0)
				goto error;
			union Object * destobject = hilbert_object_retrieve(dest, destkindhandle, HILBERT_TYPE_KIND);
			if ((destobject == NULL) || (!(destobject->generic.type & HILBERT_TYPE_EXTERNAL))) {
				errcode = HILBERT_ERR_INVALID_MAPPING;
				goto error;
			}
			struct ExternalKind * destkind = &destobject->external_kind;
			size_t destparamhandle = hilbert_ivector_get( &dest->paramhandles, destkind->paramindex );
			if (destparamhandle != arghandle) {
				errcode = HILBERT_ERR_INVALID_MAPPING;
				goto error;
			}
			const HilbertHandle * test = hilbert_pmap_post( &param->handle_map, destkindhandle );
			if (test != NULL) {
				errcode = HILBERT_ERR_MAPPING_CLASH;
				goto error;
			}
			assert (((srckind->type ^ destkind->type) & HILBERT_TYPE_VKIND) == 0);
			if ( hilbert_pmap_add( &param->handle_map, destkindhandle, srckindhandle ) != 0 ) {
				errcode = HILBERT_ERR_NOMEM;
				goto error;
			}
		} else {
			/* map to new kind */
			HilbertHandle destkindhandle = hilbert_ovector_count( &dest->objects );
			union Object * destkind = malloc(sizeof(*destkind));
			if (destkind == NULL) {
				errcode = HILBERT_ERR_NOMEM;
				goto error;
			}
			if ( hilbert_ovector_pushback( &dest->objects, destkind ) != 0 ) {
				free(destkind);
				errcode = HILBERT_ERR_NOMEM;
				goto error;
			}
			if ( hilbert_ivector_pushback( &dest->kindhandles, destkindhandle ) != 0 ) {
				errcode = HILBERT_ERR_NOMEM;
				goto error;
			}
			destkind->external_kind = (struct ExternalKind) {
				.type = srcobject->generic.type | HILBERT_TYPE_EXTERNAL,
				.equivalence_class = NULL,
				.paramindex = paramindex
			};
			if ( hilbert_pmap_add( &param->handle_map, destkindhandle, srckindhandle ) != 0 ) {
				errcode = HILBERT_ERR_NOMEM;
				goto error;
			}
		}
	}

	/* coarsen kind equivalence relation in dest to become compatible with src */
	EQCSet backup;
	errcode = eqc_backup_init( &backup, dest, &param->handle_map );
	if ( errcode != 0 ) {
		errcode = HILBERT_ERR_NOMEM;
		goto nobackupmem;
	}
	for ( void * i = hilbert_pmap_iterator_start( &param->handle_map ); i != NULL; i = hilbert_pmap_iterator_next( &param->handle_map, i ) ) {
		struct ParamMapEntry entry = hilbert_pmap_iterator_get( &param->handle_map, i );
		if ( hilbert_iset_contains( &already_handled, entry.post ) ) {
			continue;
		}
		union Object * srcobject = hilbert_object_retrieve(src, entry.post, HILBERT_TYPE_KIND);
		assert (srcobject != NULL);
		struct Kind * srckind = &srcobject->kind;
		if (srckind->equivalence_class != NULL) {
			for ( void * j = hilbert_iset_iterator_start( srckind->equivalence_class ); j != NULL; j = hilbert_iset_iterator_next( srckind->equivalence_class, j ) ) {
				HilbertHandle srckindhandle = hilbert_iset_iterator_get( srckind->equivalence_class, j );
				const HilbertHandle * destkindhandle = hilbert_pmap_pre( &param->handle_map, srckindhandle );
				assert (destkindhandle != NULL);
				errcode = hilbert_kind_identify_nocheck(dest, entry.pre, *destkindhandle);
				assert ((errcode != HILBERT_ERR_INVALID_MODULE)
						&& (errcode != HILBERT_ERR_IMMUTABLE)
						&& (errcode != HILBERT_ERR_INVALID_HANDLE));
				if (errcode != 0) {
					eqc_backup_restore( dest, &param->handle_map, &backup );
					goto iderror;
				}
			}
		}
	}

	errcode = 0;

	eqc_backup_fini( &backup );
iderror:
nobackupmem:
error:
	hilbert_iset_fini( &already_handled );
noahsetmem:
	return errcode;
}

/**
 * Loads functors from a source module into a destination module.
 *
 * @param dest Pointer to destination module, assumed to be locked.
 * @param src Pointer to source module, assumed to be locked.
 * @param argv Pointer to array of arguments to the parameters of the module pointed to by <code>src</code>.
 * 	If the number of elements in the array does not match the number of parameters, the behaviour is undefined.
 * @param mapper Pointer to parameter handle to argument handle mapper function.
 * 	If the number of parameters is zero, <code>mapper</code> may be <code>NULL</code>.
 * @param userdata Pointer to user-defined data passed as an argument to the userdata parameter of <code>mapper</code>.
 * @param param Pointer to the new parameter.
 * @param paramindex Index of the new parameter in <code>dest</code>.
 *
 * Warning: this function adds elements to <code>dest->objects</code> and <code>dest->functorhandles</code> without deleting them on error.
 * It is up to the caller to do that.
 *
 * @return On success, <code>0</code> is returned. On error, a nonzero value is returned.
 */
static int load_functors(HilbertModule * restrict dest, HilbertModule * restrict src, const HilbertHandle * restrict argv, HilbertMapperCallback mapper, void * userdata, struct Param * param, size_t paramindex) {
	assert (dest != NULL);
	assert (src != NULL);
	assert ( ( hilbert_ivector_count( &src->paramhandles ) == 0 ) || ( argv != NULL ) );
	assert ( ( hilbert_ivector_count( &src->paramhandles ) == 0 ) || ( mapper != NULL ) );
	assert (param != NULL);

	int errcode;

	/* Inspect all source functors */
	for ( void * i = hilbert_ivector_iterator_start( &src->functorhandles ); i != NULL; i = hilbert_ivector_iterator_next( &src->functorhandles, i ) ) {
		HilbertHandle srcfunctorhandle = hilbert_ivector_iterator_get( &src->functorhandles, i );
		union Object * srcobject = hilbert_ovector_get( &src->objects, srcfunctorhandle );
		assert (srcobject->generic.type & HILBERT_TYPE_FUNCTOR);
		if (srcobject->generic.type & HILBERT_TYPE_EXTERNAL) {
			/* map to existing functor */
			struct ExternalBasicFunctor * srcfunctor = &srcobject->external_basic_functor;
			HilbertHandle arghandle = argv[srcfunctor->paramindex];
			HilbertHandle destfunctorhandle = mapper(dest, src, srcfunctorhandle, userdata, &errcode);
			if (errcode != 0)
				goto error;
			union Object * destobject = hilbert_object_retrieve(dest, destfunctorhandle, HILBERT_TYPE_FUNCTOR);
			if ((destobject == NULL) || (!(destobject->generic.type & HILBERT_TYPE_EXTERNAL))) { // FIXME: abbreviation, definitions?
				errcode = HILBERT_ERR_INVALID_MAPPING;
				goto error;
			}
			struct ExternalBasicFunctor * destfunctor = &destobject->external_basic_functor;
			size_t destparamhandle = hilbert_ivector_get( &dest->paramhandles, destfunctor->paramindex );
			if (destparamhandle != arghandle) {
				errcode = HILBERT_ERR_INVALID_MAPPING;
				goto error;
			}
			const HilbertHandle * test = hilbert_pmap_post( &param->handle_map, destfunctorhandle );
			if (test != NULL) {
				errcode = HILBERT_ERR_MAPPING_CLASH;
				goto error;
			}
			if ( hilbert_pmap_add( &param->handle_map, destfunctorhandle, srcfunctorhandle ) != 0 ) {
				errcode = HILBERT_ERR_NOMEM;
				goto error;
			}
		} else {
			/* map to new functor */
			HilbertHandle destfunctorhandle = hilbert_ovector_count( &dest->objects);
			union Object * destobject = malloc(sizeof(*destobject));
			if (destobject == NULL) {
				errcode = HILBERT_ERR_NOMEM;
				goto error;
			}
			const struct BasicFunctor * srcfunctor = &srcobject->basic_functor;
			destobject->external_basic_functor = (struct ExternalBasicFunctor) { .type = srcfunctor->type | HILBERT_TYPE_EXTERNAL, .input_kinds = NULL, .paramindex = paramindex }; // FIXME: abbrev, def?
			struct ExternalBasicFunctor * destfunctor = &destobject->external_basic_functor;
			if ( hilbert_ovector_pushback( &dest->objects, destobject ) != 0 ) {
				free(destfunctor);
				errcode = HILBERT_ERR_NOMEM;
				goto error;
			}
			if ( hilbert_ivector_pushback( &dest->functorhandles, destfunctorhandle ) != 0 ) {
				errcode = HILBERT_ERR_NOMEM;
				goto error;
			}
			const HilbertHandle * handle = hilbert_pmap_pre( &param->handle_map, srcfunctor->result_kind );
			assert (handle != NULL);
			assert (hilbert_object_retrieve(dest, *handle, HILBERT_TYPE_KIND) != NULL);
			assert (!(hilbert_object_retrieve(dest, *handle, HILBERT_TYPE_KIND)->generic.type & HILBERT_TYPE_VKIND));
			destfunctor->result_kind = *handle;
			destfunctor->place_count = srcfunctor->place_count;
			assert (destfunctor->place_count < SIZE_MAX / sizeof(*destfunctor->input_kinds));
			size_t allocsize = destfunctor->place_count * sizeof(*destfunctor->input_kinds);
			if (allocsize != 0) {
				destfunctor->input_kinds = malloc(allocsize);
				if (destfunctor->input_kinds == NULL) {
					errcode = HILBERT_ERR_NOMEM;
					goto error;
				}
			}
			for (size_t i = 0; i != destfunctor->place_count; ++i) {
				const HilbertHandle * handle = hilbert_pmap_pre( &param->handle_map, srcfunctor->input_kinds[i] );
				assert (handle != NULL);
				assert (hilbert_object_retrieve(dest, *handle, HILBERT_TYPE_KIND) != NULL);
				destfunctor->input_kinds[i] = *handle;
			}
			if ( hilbert_pmap_add( &param->handle_map, destfunctorhandle, srcfunctorhandle ) != 0 ) {
				errcode = HILBERT_ERR_NOMEM;
				goto error;
			}
		}
	}

	errcode = 0;

error:
	return errcode;
}

HilbertHandle hilbert_module_param(HilbertModule * restrict dest, HilbertModule * restrict src, size_t argc,
		const HilbertHandle * restrict argv, HilbertMapperCallback mapper, void * userdata, int * restrict errcode) {
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

	if ( hilbert_ivector_count( &src->paramhandles ) != argc ) {
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

	size_t oldcount = hilbert_ovector_count( &dest->objects );
	result = oldcount;

	if ( hilbert_ovector_pushback( &dest->objects, param ) != 0 ) {
		*errcode = HILBERT_ERR_NOMEM;
		goto noobjectmem;
	}

	size_t paramindex = hilbert_ivector_count( &dest->paramhandles );
	size_t oldkcount = hilbert_ivector_count( &dest->kindhandles );
	*errcode = load_kinds(dest, src, argv, mapper, userdata, &param->param, paramindex);
	if (*errcode != 0)
		goto kindloaderror;
	size_t oldfcount = hilbert_ivector_count( &dest->functorhandles );
	*errcode = load_functors(dest, src, argv, mapper, userdata, &param->param, paramindex); // FIXME: abbrev, def?
	if (*errcode != 0)
		goto functorloaderror;
	
	if ( hilbert_ivector_pushback( &dest->paramhandles, result ) != 0 ) {
		*errcode = HILBERT_ERR_NOMEM;
		goto noparamhandlemem;
	}

	*errcode = set_dependency(dest, src);
	if (*errcode != 0)
		goto deperror;

	goto success;

deperror:
noparamhandlemem:
functorloaderror:
	if ( hilbert_ivector_downsize( &dest->functorhandles, oldfcount ) != 0 ) {
		*errcode = HILBERT_ERR_INTERNAL;
	}
kindloaderror:
	if ( hilbert_ivector_downsize( &dest->kindhandles, oldkcount ) != 0 ) {
		*errcode = HILBERT_ERR_INTERNAL;
	}
	size_t newcount = hilbert_ovector_count( &dest->objects );
	for (size_t i = oldcount + 1; i < newcount; ++i) { /* param is freed below */
		hilbert_object_free( hilbert_ovector_get( &dest->objects, i ) );
	}
	if ( hilbert_ovector_downsize( &dest->objects, oldcount ) != 0 ) {
		*errcode = HILBERT_ERR_INTERNAL;
	}
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
		const HilbertHandle * restrict argv, HilbertMapperCallback mapper, void * userdata, int * restrict errcode) {
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

	if ( hilbert_ivector_count( &src->paramhandles ) != argc ) {
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

	size_t oldcount = hilbert_ovector_count( &dest->objects );
	result = oldcount;

	if ( hilbert_ovector_pushback( &dest->objects, param ) != 0 ) {
		*errcode = HILBERT_ERR_NOMEM;
		goto noobjectmem;
	}

	size_t paramindex = hilbert_ivector_count( &dest->paramhandles );
	size_t oldkcount = hilbert_ivector_count( &dest->kindhandles );
	*errcode = load_kinds(dest, src, argv, mapper, userdata, &param->param, paramindex);
	if (*errcode != 0)
		goto kindloaderror;
	size_t oldfcount = hilbert_ivector_count( &dest->functorhandles );
	*errcode = load_functors(dest, src, argv, mapper, userdata, &param->param, paramindex); // FIXME: abbrev, def?
	if (*errcode != 0)
		goto functorloaderror;
	// FIXME: statements
	
	if ( hilbert_ivector_pushback( &dest->paramhandles, result ) != 0 ) {
		*errcode = HILBERT_ERR_NOMEM;
		goto noparamhandlemem;
	}

	*errcode = set_dependency(dest, src);
	if (*errcode != 0)
		goto deperror;

	goto success;

deperror:
noparamhandlemem:
functorloaderror:
	if ( hilbert_ivector_downsize( &dest->functorhandles, oldfcount ) != 0 ) {
		*errcode = HILBERT_ERR_INTERNAL;
	}
kindloaderror:
	if ( hilbert_ivector_downsize( &dest->kindhandles, oldkcount ) != 0 ) {
		*errcode = HILBERT_ERR_INTERNAL;
	}
	size_t newcount = hilbert_ovector_count( &dest->objects );
	for (size_t i = oldcount + 1; i < newcount; ++i) { /* param is freed below */
		hilbert_object_free( hilbert_ovector_get( &dest->objects, i ) );
	}
	if ( hilbert_ovector_downsize( &dest->objects, oldcount ) != 0 ) {
		*errcode = HILBERT_ERR_INTERNAL;
	}
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
