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

#ifndef HILBERT_PRIVATE_H__
#define HILBERT_PRIVATE_H__

#include"hilbert.h"

#include"cl/pmap.h"
#include"cl/iset.h"
#include"cl/mset.h"
#include"cl/ivector.h"
#include"cl/ovector.h"

#include"threads/hthreads.h"

/**
 * Generic object type.
 */
struct Generic {
	/**
	 * Object type.
	 */
	unsigned int type;
};

/**
 * Kind.
 */
struct Kind {
	/**
	 * Kind type (#HILBERT_TYPE_KIND set).
	 */
	unsigned int type;

	/**
	 * Equivalence class.
	 * If this kind is a singleton, the equivalence class is <code>NULL</code>.
	 */
	IndexSet * equivalence_class;
};

/**
 * External kind (parameter, import, or export)
 */
struct ExternalKind {
	/**
	 * External kind type (#HILBERT_TYPE_KIND | #HILBERT_TYPE_EXTERNAL)
	 */
	unsigned int type;

	/**
	 * Equivalence class.
	 * If this kind is a singleton, the equivalence class is <code>NULL</code>.
	 */
	IndexSet * equivalence_class;

	/**
	 * Index into <code>#struct HilbertModule::paramhandles</code>
	 */
	size_t paramindex;
};

/**
 * Parameter.
 */
struct Param {
	/**
	 * Parameter type (#HILBERT_TYPE_PARAM)
	 */
	unsigned int type;

	/**
	 * Module with which we parameterise.
	 */
	struct HilbertModule * module;

	/**
	 * Map mapping local handles to handles of <code>module</code>.
	 */
	ParamMap * handle_map;
};

/**
 * Object.
 */
union Object {
	struct Generic generic;
	struct Kind kind;
	struct ExternalKind external_kind;
	struct Param param;
};

/**
 * Frees a kind.
 *
 * @param kind Pointer to a previously allocated kind.
 */
static inline void hilbert_kind_free(union Object * kind) {
	/* kind->equivalence_class handled in hilbert_module_free() */
	free(kind);
}

/**
 * Frees a parameter.
 *
 * @param param Pointer to a previously allocated parameter.
 */
static inline void hilbert_param_free(union Object * param) {
	hilbert_pmap_del(param->param.handle_map);
	free(param);
}

/**
 * Frees an object.
 *
 * @param pointer to constituent to be freed.
 */
static inline void hilbert_object_free(union Object * object) {
	assert (object != NULL);
	unsigned int type = object->generic.type;
	switch (type) {
		case HILBERT_TYPE_KIND:
		case HILBERT_TYPE_KIND | HILBERT_TYPE_EXTERNAL:
			hilbert_kind_free(object);
			break;
		case HILBERT_TYPE_PARAM:
			hilbert_param_free(object);
			break;
		/* FIXME: case ... */
		default:
			assert (2 + 2 == 5);
			break;
	}
}

/**
 * Private Hilbert module structure.
 */
struct HilbertModule {
	/**
	 * Module type.
	 */
	enum HilbertModuleType type;

	/**
	 * Mutual exclusion device.
	 */
	HILBERT_MUTEX_DECL(mutex);

	/**
	 * Whether this module is immutable.
	 */
	unsigned int immutable: 1;

	/**
	 * Whether user has requested this module to be freed.
	 */
	unsigned int freeable: 1;

	/**
	 * Ancillary (user set) data.
	 */
	void * ancillary;

	/**
	 * Module constituents.
	 */
	ObjectVector * objects;

	/**
	 * Kind handles.
	 */
	IndexVector * kindhandles;

	/**
	 * Parameter handles.
	 */
	IndexVector * paramhandles;

	/**
	 * Set of modules this module depends on.
	 */
	ModuleSet * dependencies;

	/**
	 * Set of modules depending on this module.
	 */
	ModuleSet * reverse_dependencies;
};

/**
 * Retrieves an object if it has the specified type.
 *
 * @param module pointer to a Hilbert module.
 * 	Any necessary locking on the module must be done by the caller!
 * @param handle object handle.
 * @param typeflags sought type flags.
 *
 * @return If <code>handle</code> is in the range of <code>module->objects</code> and the specified object's type has at least one bit in common with <code>typeflags</code>, a pointer to that object is returned.
 * 	Otherwise, <code>NULL</code> is returned.
 */
static inline union Object * hilbert_object_retrieve(const struct HilbertModule * module, HilbertHandle handle, unsigned int typeflags) {
	assert (module != NULL);

	size_t numobjects = hilbert_ovector_count(module->objects);
	if (handle >= numobjects)
		return NULL;
	union Object * result = hilbert_ovector_get(module->objects, handle);
	if (!(result->generic.type & typeflags))
		return NULL;
	return result;
}

/**
 * Kind identification without locks and checks.
 *
 * @param module Pointer to a Hilbert module.
 * @param kindhandle1 Kind handle.
 * @param kindhandle2 Kind handle.
 *
 * @return On error, a negative value is returned, which may be one of the following error codes:
 * 	- <code>HILBERT_ERR_NOMEM</code>:
 * 		There was not enough memory to perform the operation.
 * 	- <code>HILBERT_ERR_INVALID_HANDLE</code>
 * 		One of <code>kind1</code>, <code>kind2</code> is not a valid kind handle in the module pointed
 * 		to by <code>module</code>.
 */
static inline int hilbert_kind_identify_nocheck(struct HilbertModule * module, HilbertHandle kindhandle1,
		HilbertHandle kindhandle2) {
	assert (module != NULL);

	int errcode;

	union Object * object1 = hilbert_object_retrieve(module, kindhandle1, HILBERT_TYPE_KIND);
	union Object * object2 = hilbert_object_retrieve(module, kindhandle2, HILBERT_TYPE_KIND);
	if ((object1 == NULL) || (object2 == NULL)) {
		errcode = HILBERT_ERR_INVALID_HANDLE;
		goto invalidhandle;
	}
	/* check if already equivalent */
	errcode = 0;
	if (object1 == object2)
		goto equivalent;
	if ((object1->kind.equivalence_class != NULL)
			&& (object1->kind.equivalence_class == object2->kind.equivalence_class))
		goto equivalent;

	errcode = HILBERT_ERR_NOMEM;
	IndexSet * equivalence_class = hilbert_iset_new();
	if (equivalence_class == NULL)
		goto noeqcmem;
	/* create union of equivalence classes */
	if (object1->kind.equivalence_class != NULL) {
		if (hilbert_iset_addall(equivalence_class, object1->kind.equivalence_class) != 0)
			goto nounionmem;
	}
	if (object2->kind.equivalence_class != NULL) {
		if (hilbert_iset_addall(equivalence_class, object2->kind.equivalence_class) != 0)
			goto nounionmem;
	}
	if (hilbert_iset_add(equivalence_class, kindhandle1) != 0)
		goto nounionmem;
	if (hilbert_iset_add(equivalence_class, kindhandle2) != 0)
		goto nounionmem;
	/* delete old equivalence classes */
	if (object1->kind.equivalence_class != NULL)
		hilbert_iset_del(object1->kind.equivalence_class);
	if (object2->kind.equivalence_class != NULL)
		hilbert_iset_del(object2->kind.equivalence_class);
	/* update all affected equivalence classes */
	for (IndexSetIterator i = hilbert_iset_iterator_new(equivalence_class); hilbert_iset_iterator_hasnext(&i);) {
		HilbertHandle handle = hilbert_iset_iterator_next(&i);
		union Object * object = hilbert_object_retrieve(module, handle, ~0U);
		assert (object->generic.type & HILBERT_TYPE_KIND);
		object->kind.equivalence_class = equivalence_class;
	}

	errcode = 0;
	goto success;

nounionmem:
	hilbert_iset_del(equivalence_class);
success:
noeqcmem:
equivalent:
invalidhandle:
	return errcode;
}

#endif
