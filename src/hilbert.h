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

#ifndef HILBERT_H__
#define HILBERT_H__

#include<limits.h>
#include<stddef.h>

/**
 * Hilbert module types.
 */
enum HilbertModuleType {
	HILBERT_INTERFACE_MODULE,
	HILBERT_PROOF_MODULE
};

/**
 * Opaque Hilbert module type.
 */
typedef struct HilbertModule HilbertModule;

/**
 * Handle type for basic module constituents.
 *
 * The handle type is an unsigned integer type.
 */
typedef size_t HilbertHandle;

/**
 * Hilbert expression types.
 */
enum HilbertExpressionType {
	HILBERT_FINISHED_EXPRESSION,
	HILBERT_UNFINISHED_EXPRESSION
};

/**
 * Opaque Hilbert expression type.
 */
typedef struct HilbertExpression HilbertExpression;

/**
 * Maximum integer value representable as a <code>#HilbertHandle</code>.
 */
#define HILBERT_HANDLE_MAX SIZE_MAX

/**
 * Function pointer type for mapping objects between modules.
 * It is required by library functions responsible for parameterising, importing, and exporting Hilbert interface modules.
 *
 * @param dest Pointer to a Hilbert module that is the target of a parameterisation, an import, or an export.
 * @param src Pointer to a Hilbert module that is the source of a parameterisation, an import, or an export.
 * @param srcObject Hilbert handle of an object in <code>src</code> whose corresponding handle in <code>dest</code> is sought.
 * @param userdata Pointer to user-defined data.
 * @param errcode Pointer to an integer used to convey a user-defined error code.
 *
 * @return On error, the return value is unspecified and a user-defined positive error code is stored in <code>*errcode</code>.
 * 	It is required that a positive value be stored in <code>*errcode</code> as the Hilbert kernel library uses negative integers for error codes.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>, and the object handle in <code>dest</code> corresponding to the object handle <code>srcObject</code> is returned.
 */
typedef HilbertHandle (*HilbertMapperCallback)(HilbertModule * restrict dest, HilbertModule * restrict src, HilbertHandle srcObject, void * userdata, int * restrict errcode);

/**
 * Error codes.
 *
 * Most Hilbert kernel library functions convey error conditions in the form of error codes.
 * Library error codes are negative integers. A value of zero indicates no error.
 * Occasionally, a library function may convey user-defined positive error codes resulting from errors in user-provided callback functions.
 *
 * Each library function conveys error conditions with error codes only from a subset of the error codes
 * listed below. The allowed error codes are listed in the description of each function. One exception to this rule
 * is the special error code <code>#HILBERT_ERR_INTERNAL</code>, which may be provided by all library functions
 * conveying errors without being mentioned explicitly.
 */

/**
 * Error code to indicate that a request could not be fulfilled due to lack of memory.
 */
#define HILBERT_ERR_NOMEM           (-1)

/**
 * Error code to indicate that an operation allowed on mutable modules only has been attempted on an immutable module,
 * or an operation allowed on immutable modules only has been attempted on a mutable module.
 */
#define HILBERT_ERR_IMMUTABLE       (-2)

/**
 * Error code to indicate that an operation allowed on interface modules only has been attempted on a proof module,
 * or an operation allowed on proof modules only has been attempted on an interface module.
 */
#define HILBERT_ERR_INVALID_MODULE  (-3)

/**
 * Error code to indicate that an object handle provided to a library function does not correspond to an actual object,
 * or the object it corresponds to has the wrong type.
 */
#define HILBERT_ERR_INVALID_HANDLE  (-4)

/**
 * Error code to indicate a mismatch between an expected and a provided number of objects.
 */
#define HILBERT_ERR_COUNT_MISMATCH  (-5)

/**
 * Error code to indicate an error with an object handle provided by a <code>#HilbertMapperCallback</code>.
 */
#define HILBERT_ERR_INVALID_MAPPING (-6)

/**
 * Error code to indicate a map provided by <code>#HilbertMapperCallback</code> is not one-to-one.
 */
#define HILBERT_ERR_MAPPING_CLASH   (-7)

/**
 * Error code to indicate a map provided by <code>#HilbertMapperCallback</code> does not respect
 * kind equivalence classes.
 */
#define HILBERT_ERR_NO_EQUIVALENCE  (-8)

/**
 * Error code to indicate that a Hilbert expression provided to a library function is invalid.
 */
#define HILBERT_ERR_INVALID_EXPR    (-9)

/**
 * Error code to indicate a serious internal error in the Hilbert kernel library.
 *
 * Once a Hilbert kernel function conveys this error code, all subsequent calls to functions in the Hilbert kernel library,
 * including concurrent calls not yet completed, may produce undefined behaviour.
 *
 * Nevertheless, an implementation should avoid eliciting undefined behaviour if possible even in the face of a serious internal error.
 */
#define HILBERT_ERR_INTERNAL       (-99)

/**
 * Object type flags.
 *
 * Each object in a Hilbert module has a set of logically OR'd type flags.
 *
 * @sa #hilbert_object_gettype()
 */

/**
 * Flag to indicate that the corresponding object is derived from another object external to the module.
 * This flag is mutually exclusive with <code>#HILBERT_TYPE_PARAM</code> and <code>#HILBERT_TYPE_VAR</code>.
 */
#define HILBERT_TYPE_EXTERNAL 0x0001u

/**
 * Flag to indicate that the corresponding object is a kind.
 * This flag is mutually exclusive with <code>#HILBERT_TYPE_PARAM</code> and <code>#HILBERT_TYPE_VAR</code>.
 *
 * @sa #hilbert_kind_create()
 */
#define HILBERT_TYPE_KIND     0x0002u

/**
 * Flag to indicate that the corresponding object is a parameter.
 * This flag is mutually exclusive with <code>#HILBERT_TYPE_EXTERNAL</code>, <code>#HILBERT_TYPE_VAR</code> and <code>#HILBERT_TYPE_KIND</code>.
 *
 * @sa #hilbert_module_param()
 */
#define HILBERT_TYPE_PARAM    0x0004u

/**
 * Flag to indicate that the corresponding object is a variable kind,
 * that is, a kind which cannot be the resultant type of a functor application.
 * This flag requires <code>#HILBERT_TYPE_KIND</code>, and is mutually exclusive with <code>#HILBERT_TYPE_PARAM</code> and <code>#HILBERT_TYPE_VAR</code>.
 */
#define HILBERT_TYPE_VKIND    0x0008u

/**
 * Flag to indicate that the corresponding object is a variable.
 * This flag is mutually exclusive with <code>#HILBERT_TYPE_EXTERNAL</code>, <code>#HILBERT_TYPE_KIND</code>, <code>#HILBERT_TYPE_PARAM</code> and <code>#HILBERT_TYPE_VKIND</code>.
 */
#define HILBERT_TYPE_VAR      0x0010u

/**
 * Flag to indicate that the corresponding object is a functor.
 * This flag is mutually exclusive with <code>#HILBERT_TYPE_KIND</code>, <code>#HILBERT_TYPE_PARAM</code>, <code>#HILBERT_TYPE_VAR</code> and <code>#HILBERT_TYPE_VKIND</code>.
 */
#define HILBERT_TYPE_FUNCTOR  0x0020u

/**
 * Creates a new Hilbert module.
 *
 * @param type The type of the module. Can be one of the following:
 * 	- <code>#HILBERT_INTERFACE_MODULE</code>:
 * 		Interface modules may be used as parameters in other interface modules
 *		or imported into or exported from proof modules.
 *	- <code>#HILBERT_PROOF_MODULE</code>:
 *		Proof modules may contain proofs and may import or export interface modules.
 *	It is an error if the type is not among the choices listed above.
 *
 * @return On success, a pointer to a new Hilbert module is returned.
 * 	On error (e.g., insufficient memory), <code>NULL</code> is returned.
 */
HilbertModule * hilbert_module_create(enum HilbertModuleType type);

/**
 * Frees a Hilbert module previously created by <code>#hilbert_module_create()</code>.
 *
 * This must be the last function called on a module pointer.
 * It must be called after all other functions called on the module pointer have returned.
 * Otherwise, the behaviour is undefined.
 *
 * @param module Pointer to a <code>#HilbertModule</code> previously returned by a successful call to <code>#hilbert_module_create()</code>.
 */
void hilbert_module_free(HilbertModule * module);

/**
 * Obtains the type of a Hilbert module.
 *
 * @param module Pointer to a code>#HilbertModule</code> previously returned by a successful call to <code>#hilbert_module_create()</code>.
 *
 * @return The <code>#HilbertModuleType</code> of the module pointed to by <code>module</code> is returned.
 *
 * @sa hilbert_module_create()
 */
enum HilbertModuleType hilbert_module_gettype(HilbertModule * module);

/**
 * Makes an interface module immutable.
 *
 * No new basic constitutents can be added to an immutable module.
 * Only immutable interface modules can be imported or exported,
 * or used as parameters.
 *
 * @param module Pointer to a code>#HilbertModule</code> previously returned by a successful call to <code>#hilbert_module_create()</code>.
 * 	The module must be of type <code>#HILBERT_INTERFACE_MODULE</code>.
 *
 * @return On success, <code>0</code> is returned.
 * 	On error, a negative value is returned, which may be one of the following error codes:
 * 	- <code>#HILBERT_ERR_INVALID_MODULE</code>:
 * 		The provided module is not an interface module.
 * 	- <code>#HILBERT_ERR_IMMUTABLE</code>:
 * 		The provided module is already immutable.
 *
 * @sa hilbert_module_gettype()
 * @sa hilbert_module_isimmutable()
 */
int hilbert_module_makeimmutable(HilbertModule * module);

/**
 * Checks whether a Hilbert module is immutable.
 *
 * @param module Pointer to a code>#HilbertModule</code> previously returned by a successful call to <code>#hilbert_module_create()</code>.
 * @param errcode Pointer to an integer used to convey an error code.
 *
 * @return On error, a negative value is stored in <code>*errcode</code> and the return value is unspecified.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>, and the return value is as follows.
 * 	If the module is not of type <code>#HILBERT_INTERFACE_MODULE</code>, <code>0</code> is returned.
 * 	Otherwise, a non-zero value is returned
 * 	if and only if a previous call to <code>hilbert_module_makeimmutable()</code> with the module was successful.
 *
 * @sa hilbert_module_makeimmutable()
 * @sa hilbert_module_gettype()
 */
int hilbert_module_isimmutable(HilbertModule * restrict module, int * restrict errcode);

/**
 * Sets ancillary data for a module.
 * Users may install a pointer to arbitrary ancillary data in a module.
 * This can be helpful for adding convenient support in higher level libraries.
 *
 * @param module Pointer to a code>#HilbertModule</code> previously returned by a successful call to <code>#hilbert_module_create()</code>.
 * @param newdata Pointer to new ancillary data.
 * @param olddata Pointer to a writable location able to hold a pointer, or <code>NULL</code>.
 *
 * @return On success, <code>0</code> is returned.
 * 	If <code>olddata</code> is not <code>NULL</code>, the previous ancillary data pointer
 * 	is assigned to <code>*olddata</code>.
 * 	If this is the first call to this function with <code>module</code>, the previous ancillary
 * 	data pointer is <code>NULL</code>.
 * 	On error, a negative value is returned.
 * 	The ancillary data pointer of the module will not be changed.
 * 	If <code>olddata</code> is not <code>NULL</code>, the value of <code>*olddata</code> is unspecified.
 *
 * @sa hilbert_module_getancillary()
 */
int hilbert_module_setancillary(HilbertModule * module, void * newdata, void ** olddata);

/**
 * Obtains pointer to ancillary data to the module.
 *
 * @param module Pointer to a code>#HilbertModule</code> previously returned by a successful call to <code>#hilbert_module_create()</code>.
 * @param data Pointer to a writable location able to hold a pointer.
 *
 * @return On success, <code>0</code> is returned.
 * 	The ancillary data pointer set by the last successful call to <code>hilbert_module_setancillary()</code> is assigned to <code>*data</code>.
 * 	If there was no such call, <code>NULL</code> is assigned.
 * 	On error, a negative value is returned, and the value of <code>*data</code> is unspecified.
 *
 * @sa hilbert_module_setancillary()
 */
int hilbert_module_getancillary(HilbertModule * module, void ** data);

/**
 * Creates a new Hilbert kind in the specified interface module.
 *
 * @param module Pointer to a Hilbert interface module.
 * @param errcode Pointer to an integer used to convey an error code.
 *
 * @return On success, <code>0</code> is stored in <code>*errcode</code> and a handle for the new kind is returned.
 * 	On error, the return value is unspecified,
 * 	and a negative value is stored in <code>*errcode</code>, which may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_NOMEM</code>:
 * 			There was not enough memory to create the new kind.
 * 		- <code>#HILBERT_ERR_INVALID_MODULE</code>:
 * 			The module pointed to by <code>module</code> is a proof module.
 * 		- <code>#HILBERT_ERR_IMMUTABLE</code>:
 * 			The module pointed to by <code>module</code> is immutable.
 */
HilbertHandle hilbert_kind_create(HilbertModule * restrict module, int * restrict errcode);

/**
 * Creates a new Hilbert variable kind in the specified interface module.
 *
 * @param module Pointer to a Hilbert interface module.
 * @param errcode Pointer to an integer used to convey an error code.
 *
 * @return On success, <code>0</code> is stored in <code>*errcode</code> and a handle for the new variable kind is returned.
 * 	On error, the return value is unspecified,
 * 	and a negative value is stored in <code>*errcode</code>, which may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_NOMEM</code>:
 * 			There was not enough memory to create the new variable kind.
 * 		- <code>#HILBERT_ERR_INVALID_MODULE</code>:
 * 			The module pointed to by <code>module</code> is a proof module.
 * 		- <code>#HILBERT_ERR_IMMUTABLE</code>:
 * 			The module pointed to by <code>module</code> is immutable.
 */
HilbertHandle hilbert_vkind_create(HilbertModule * restrict module, int * restrict errcode);

/**
 * Creates an alias of an existing Hilbert kind.
 * The new alias kind will be equivalent to the specified kind.
 * It will be a variable kind if and only if the existing kind is also a variable kind.
 *
 * @param module Pointer to a Hilbert module.
 * @param kind Kind handle of a kind in <code>module</code>.
 * @param errcode Pointer to an integer used to convey an error code.
 *
 * @return On success, <code>0</code> is stored in <code>*errcode</code> and a handle for the new alias kind is returned.
 * 	On error, the return value is unspecified,
 * 	and a negative value is stored in <code>*errcode</code>, which may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_NOMEM</code>:
 * 			There was not enough memory to create the new kind alias.
 * 		- <code>#HILBERT_ERR_INVALID_HANDLE</code>:
 * 			<code>kind</code> is not a valid kind handle for <code>module</code>.
 * 		- <code>#HILBERT_ERR_IMMUTABLE</code>:
 * 			The module pointed to by <code>module</code> is immutable.
 */
HilbertHandle hilbert_kind_alias(HilbertModule * restrict module, HilbertHandle kind, int * restrict errcode);

/**
 * Identifies two Hilbert kinds in an interface module.
 * If the two specified kinds are already equivalent (for example, if they are equal),
 * no operation is performed.
 *
 * @param module Pointer to a Hilbert interface module.
 * @param kind1 Kind handle of a kind in <code>module</code>.
 * @param kind2 Kind handle of a kind in <code>module</code>.
 *
 * @return On success, <code>0</code> is returned.
 * 	On error, a negative value is returned, which may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_NOMEM</code>:
 * 			There was not enough memory available to identify <code>kind1</code> and <code>kind2</code>.
 * 		- <code>#HILBERT_ERR_INVALID_MODULE</code>:
 * 			The module specified by <code>module</code> is not an interface module.
 * 		- <code>#HILBERT_ERR_IMMUTABLE</code>:
 * 			The module specified by <code>module</code> is immutable.
 * 		- <code>#HILBERT_ERR_INVALID_HANDLE</code>:
 * 			At least one of <code>kind1</code> and <code>kind2</code> is not a valid kind handle,
 * 			or exactly one of <code>kind1</code> and <code>kind2</code> is a variable kind handle.
 */
int hilbert_kind_identify(HilbertModule * module, HilbertHandle kind1, HilbertHandle kind2);

/**
 * Checks whether two Hilbert kinds are equivalent.
 *
 * @param module Pointer to a Hilbert module.
 * @param kind1 Kind handle of a kind in <code>module</code>.
 * @param kind2 Kind handle of a kind in <code>module</code>.
 * @param errcode Pointer to an integer to convey an error code.
 *
 * @return On error, the return value is unspeified and a negative value is stored in <code>*errcode</code>,
 * 	which may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_INVALID_HANDLE</code>:
 * 			At least one of <code>kind1</code> and <code>kind2</code> is not a valid kind handle.
 * 	On success, <code>0</code> is stored in <code>*errcode</code> and the return value is as follows.
 * 	If the two kinds signified by <code>kind1</code> and <code>kind2</code> are equivalent,
 * 	a non-zero value is returned.
 * 	Otherwise, <code>0</code> is returned.
 */
int hilbert_kind_isequivalent(HilbertModule * restrict module, HilbertHandle kind1, HilbertHandle kind2,
		int * restrict errcode);

/**
 * Returns the equivalence class of a Hilbert kind as an array of Hilbert kinds.
 * The returned equivalence class is only a snapshot corresponding to the current state of the underlying module.
 * It does not reflect subsequent changes to the module.
 *
 * @param module Pointer to a Hilbert module.
 * @param kind Kind handle of a kind in <code>module</code>.
 * @param count Pointer to a <code>size_t</code> to convey the number of elements in the equivalence class.
 * @param errcode Pointer to an integer to convey an error code.
 *
 * @return On error, <code>NULL</code> is returned, the value of <code>*count</count> is unspecified,
 * 	and a negative value is stored in <code>*errcode</code>, which may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_NOMEM</code>:
 * 			There was not enough memory available to create a copy of the equivalence class.
 * 		- <code>#HILBERT_ERR_INVALID_HANDLE</code>:
 * 			<code>kind</code> is not a valid handle for a kind.
 * 	On success, a pointer to the first element of an array of kind handles representing the equivalence class
 * 	is returned. The array has no specific order and contains <code>kind</code>. The number of elements in the
 * 	array is stored in <code>*count</count>. Zero is stored in <code>*errcode</code>.
 * 	The returned array must be freed by the caller with #hilbert_harray_free().
 *
 * @sa hilbert_harray_free()
 */
HilbertHandle * hilbert_kind_equivalenceclass(HilbertModule * restrict module, HilbertHandle kind, size_t * restrict count,
		int * restrict errcode);

/**
 * Frees an array of Hilbert handles previously returned by a Hilbert Kernel library function,
 * releasing any resources associated with it.
 *
 * @param handle_array Pointer previously returned by a Hilbert Kernel library function.
 */
void hilbert_harray_free(HilbertHandle * handle_array);

/**
 * Creates a new variable of the specified kind in the specified module.
 *
 * @param module Pointer to a Hilbert module.
 * @param kind Kind handle.
 * @param errcode Pointer to an integer to convey an error code.
 *
 * @return On error, the return value is unspecified, and a negative value is stored in <code>*errcode</code>,
 * 	which may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_NOMEM</code>:
 * 			There was not enough memory available to create the new variable.
 * 		- <code>#HILBERT_ERR_INVALID_HANDLE</code>:
 * 			<code>kind</code> is not a valid kind handle.
 * 		- <code>#HILBERT_ERR_IMMUTABLE</code>:
 * 			The specified module is immutable.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>, and a handle for the new variable is returned.
 *
 * @sa #hilbert_module_isimmutable()
 */
HilbertHandle hilbert_var_create(HilbertModule * restrict module, HilbertHandle kind, int * restrict errcode);

/**
 * Returns the kind of a variable.
 *
 * @param module Pointer to a Hilbert module.
 * @param var Variable handle.
 * @param errcode Pointer to an integer to convey an error code.
 *
 * @return On error, the return value is unspecified, and a negative value is stored in <code>errcode</code>,
 * 	which may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_INVALID_HANDLE</code>:
 * 			<code>var</code> is not a valid variable handle for <code>module</code>.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>, and a handle for the kind of the variable (or an equivalent kind) is returned.
 */
HilbertHandle hilbert_var_getkind(HilbertModule * restrict module, HilbertHandle var, int * restrict errcode);

/**
 * Creates a new functor.
 *
 * @param module Pointer to a Hilbert interface module in which the functor is to be created.
 * @param rkind Non-variable kind handle indicating the result kind of the new functor.
 * @param count Number of input kinds of the functor (place count).
 * @param ikinds Pointer to the first element of an array of <code>count</code> kind handles. If <code>count == 0</code>, this may be <code>NULL</code>.
 * 	The corresponding kinds are the input kinds to the functor.
 * @param errcode Pointer to an integer to convey an error code.
 *
 * @return On error, the return value is unspecified, and a negative value is stored in <code>*errcode/code>,
 * 	which may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_NOMEM</code>:
 * 			There was not enough memory available to create the new functor.
 * 		- <code>#HILBERT_ERR_INVALID_MODULE</code>:
 * 			The module specified by <code>module</code> is not an interface module.
 * 		- <code>#HILBERT_ERR_IMMUTABLE</code>:
 * 			The module pointed to by <code>module</code> is immutable.
 * 		- <code>#HILBERT_ERR_INVALID_HANDLE</code>:
 * 			<code>rkind</code> is not a non-variable kind handle, or one of the elements in the array pointed to by <code>ikinds</code> is not a kind handle.
 * 	On success, <code>0</code> is stored in <code>*errcode</code> and a handle for the new functor is returned.
 */
HilbertHandle hilbert_functor_create(HilbertModule * restrict module, HilbertHandle rkind, size_t count, const HilbertHandle * restrict ikinds, int * restrict errcode);

/**
 * Returns the result kind of a functor.
 *
 * @param module Pointer to the Hilbert module in which the functor resides.
 * @param functor Functor handle.
 * @param errcode Pointer to an integer to convey an error code.
 *
 * @return On error, the return value is unspecified, and a negative value is stored in <code>*errcode</code>,
 * 	which may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_INVALID_HANDLE</code>:
 * 			<code>functor</code> is not a functor handle in <code>module</code>.
 * 	On success, <code>0</code> is stored in <code>*errcode</code> and a handle for the result kind is returned.
 */
HilbertHandle hilbert_functor_getkind(HilbertModule * restrict module, HilbertHandle functor, int * restrict errcode);

/**
 * Returns the input kinds of a functor.
 *
 * @param module Pointer to the Hilbert module in which the functor resides.
 * @param functor Functor handle.
 * @param size Pointer to a location where the place count of the functor can be stored.
 * @param errcode Pointer to an integer to convey an error code.
 *
 * @return On error, the <code>*size</code> and the return value are unspecified, and a negative value is stored in <code>*errcode/code>,
 * 	which may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_NOMEM</code>:
 * 			There was not enough memory available to complete the request.
 * 		- <code>#HILBERT_ERR_INVALID_HANDLE</code>:
 * 			<code>functor</code> is not a functor handle in <code>module</code>.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>, the place count of the functor is stored in <code>*size</code>,
 * 	and a pointer to an array of size <code>*size</code> containing the input kind handles in proper order is returned.
 * 	The returned array should be freed by the user once it is no longer needed.
 *
 * @sa #hilbert_harray_free()
 */
HilbertHandle * hilbert_functor_getinputkinds(HilbertModule * restrict module, HilbertHandle functor, size_t * restrict size, int * restrict errcode);

/**
 * Creates a new Hilbert expression.
 *
 * @param module Pointer to the Hilbert module on which the expression should be based.
 * @param head Handle of the expression head (either a variable or a functor).
 * @param count Number of subexpressions. Must match the place count of <code>head</code>, or, if <code>head</code> is a variable handle, it must be zero.
 * @param subexpr Pointer to an array of finished Hilbert expressions.
 * 	If <code>count</code> is zero, this argument may be <code>NULL</code>.
 * @param errcode Pointer to an integer to convey an error code.
 *
 * @return On error, the return value is unspecified, and a negative value is stored in <code>*errcode</code>,
 * 	which may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_NOMEM</code>:
 * 			There was not enough memory available to create the expression.
 * 		- <code>#HILBERT_ERR_INVALID_HANDLE</code>:
 * 			<code>head</code> is neither a variable handle nor a functor handle in <code>module</code>.
 * 		- <code>#HILBERT_ERR_INVALID_MODULE</code>:
 * 			One of the expressions in the array pointed to by <code>subexpr</code> is not based on <code>module</code>.
 * 		- <code>#HILBERT_ERR_COUNT_MISMATCH</code>:
 * 			<head> is a variable handle and <code>count != 0</code>, or <code>count</code> does not equal the place count of <code>head</code>.
 * 		- <code>#HILBERT_ERR_INVALID_EXPR</code>:
 * 			One of the expressions in the array pointed to by <code>subexpr</code> is not a finished expression.
 * 		- <code>#HILBERT_ERR_NO_EQUIVALENCE</code>:
 * 			The kind of one of the expressions in the array pointed to by <code>subexpr</code> is not equivalent to the corresponding input kind of <code>head</code>.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>, and a pointer to a new, finished Hilbert expression is returned. If <code>head</code> is a variable handle, it consists solely of that variable.
 * 	Otherwise, its head functor is <code>head</code> and its subexpressions are given by the expressions in the array pointed to by <code>subexpr</code>.
 *
 * @sa #hilbert_expression_free()
 */
HilbertExpression * hilbert_expression_create(HilbertModule * restrict module, HilbertHandle head, size_t count, HilbertExpression ** restrict subexpr, int * restrict errcode);

/**
 * Starts a new Hilbert expression.
 *
 * @param module Pointer to the Hilbert module on which the expression should be based.
 * @param errcode Pointer to an integer to convey an error code.
 *
 * @param On error, the return value is unspecified, and a negative value is stored on <code>*errcode</code>,
 * 	which may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_NOMEM</code>:
 * 			There was not enough memory available to start the expression.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>, and a pointer to a new, unfinished Hilbert expression of length zero is returned.
 *
 * @sa #hilbert_expression_free()
 */
HilbertExpression * hilbert_expression_start(HilbertModule * restrict module, int * restrict errcode);

/**
 * Adds a variable or a functor to an unfinished Hilbert expression.
 * If the module on which the expression is based has been freed, the behaviour is undefined.
 *
 * @param expr Pointer to an unfinished Hilbert expression.
 * @param handle Variable or functor handle to be added to the expression. The handle must be from the module <code>expr</code> is based on.
 * @param errcode Pointer to an integer to convey an error code.
 *
 * @return On error, the return value is unspecified, and a negative value is stored on <code>*errcode</code>,
 * 	which may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_NOMEM</code>:
 * 			There was not enough memory available to add the handle.
 * 		- <code>#HILBERT_ERR_INVALID_EXPR</code>:
 * 			The expression pointed to by <code>expr</code> is not unfinished.
 * 		- <code>#HILBERT_ERR_INVALID_HANDLE</code>:
 * 			<code>handle</code> is not a variable handle or a functor handle in the module <code>expr</code> is based on.
 * 		- <code>#HILBERT_ERR_NO_EQUIVALENCE</code>:
 * 			The kind of <code>handle</code> is not equivalent to the corresponding input kind.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>, and the type of <code>expr</code> after adding the handle is returned (i.e., whether the expression in now finished).
 */
enum HilbertExpressionType hilbert_expression_add(HilbertExpression * restrict expr, HilbertHandle handle, int * restrict errcode);

/**
 * Creates a Hilbert expression from an array of Hilbert handles.
 *
 * @param module Pointer to the Hilbert module on which the expression should be based.
 * @param length Length of <code>handles</code>.
 * @param handles Pointer to an array of Hilbert handles. May be <code>NULL</code> if <code>length</code> is zero.
 * 	The handles are expected in forward Polish order.
 * @param errcode Pointer to an integer to convey an error code.
 *
 * @return On error, the return value is unspecified and a negative value is stored in <code>*errcode</code>,
 * 	which may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_NOMEM</code>:
 * 			There was not enough memory available to create the expression.
 * 		- <code>#HILBERT_ERR_INVALID_HANDLE</code>:
 * 			One of the handles in the array pointed to by <code>handles</code> is not a valid functor or variable handle of <code>module</code>.
 * 		- <code>#HILBERT_ERR_NO_EQUIVALENCE</code>:
 * 			The kind of one of the handles in the array pointed to by <code>handles</code> is not equivalent to the corresponding input kind.
 * 		- <code>#HILBERT_ERR_INVALID_EXPR</code>:
 * 			There are excess handles after the expression is finished.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>, and a pointer to a new Hilbert expression with forward Polish representation as in the array pointed to by <code>handles</code> is returned.
 * 	The new expression is not necessarily finished.
 *
 * @sa #hilbert_expression_free()
 * @sa #hilbert_expression_toarray()
 */
HilbertExpression * hilbert_expression_fromarray(HilbertModule * restrict module, size_t length, const HilbertHandle * restrict handles, int * restrict errcode);

/**
 * Returns the type of a Hilbert expression.
 * If the module on which the expression is based has been freed, the behaviour is undefined.
 *
 * @param expr Pointer to a Hilbert expression.
 * @param errcode Pointer to an integer to convey an error code.
 *
 * @return On error, the return value is unspecified, and a negative value is stored in <code>*errcode</code>.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>, and the expression type (finished or unfinished) is returned.
 */
enum HilbertExpressionType hilbert_expression_gettype(HilbertExpression * restrict expr, int * restrict errcode);

/**
 * Returns the Hilbert module on which the specified expression is based.
 * If that module has already been freed, the behaviour is undefined.
 *
 * @param expr Pointer to a Hilbert expression.
 *
 * @return The module on which the expression is based is returned.
 */
HilbertModule * hilbert_expression_getmodule(HilbertExpression * expr);

/**
 * Returns the kind of a Hilbert expression.
 * If the module on which the expression is based has been freed, the behaviour is undefined.
 *
 * @param expr Pointer to a Hilbert expression.
 * @param errcode Pointer to an integer to convey an error code.
 *
 * @return On error, the return value is unspecified, and a negative value is stored in <code>*errcode</code>,
 * 	which may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_INVALID_EXPR</code>:
 * 			The expression pointed to by <code>expr</code> is not finished.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>, and a kind handle representing the kind of the specified expression is returned.
 * 	The module in which the returned handle is defined is the module the expression is based on.
 */
HilbertHandle hilbert_expression_getkind(HilbertExpression * restrict expr, int * restrict errcode);

/**
 * Returns the immediate Hilbert subexpressions of the head of the specified finished Hilbert expression.
 * If the module on which the expression is based has been freed, the behaviour is undefined.
 *
 * @param expr Pointer to a Hilbert expression.
 * @param count Pointer to a <code>size_t</code> to convey the number of subexpressions.
 * @param errcode Pointer to an integer to convey an error code.
 *
 * @return On error, the return value and <code>*count</code> are unspecified, and a negative value is stored in <code>*errcode</code>,
 * 	which may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_NOMEM</code>:
 * 			There was not enough memory available to complete the request.
 * 		- <code>#HILBERT_ERR_INVALID_EXPR</code>:
 * 			The expression pointed to by <code>expr</code> is not finished.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>, and a pointer to an array of pointers to Hilbert expressions, the subexpressions of the head of the expression pointed to by <code>expr</code>, is returned,
 * 	the size of which is stored in <code>*count</code>. If <code>*count</code> is zero, the returned value may be <code>NULL</code>. If the head of the expression pointed to by <code>expr</code> is a variable,
 * 	then <code>*count</code> is always zero, otherwise <code>*count</code> equals the place count of the head functor.
 *
 * @sa #hilbert_earray_free()
 */
HilbertExpression ** hilbert_expression_subexpressions(HilbertExpression * restrict expr, size_t * restrict count, int * restrict errcode);

/**
 * Frees an array of Hilbert expressions.
 *
 * @param count Length of the array.
 * @param expressions Pointer to an array of pointers to Hilbert expressions to be freed.
 */
void hilbert_earray_free(size_t count, HilbertExpression ** expressions);

/**
 * Returns the length of a Hilbert expression.
 * If the module on which the expression is based has been freed, the behaviour is undefined.
 *
 * @param expr Pointer to a Hilbert expression.
 * @param errcode Pointer to an integer to convey an error code.
 *
 * @return On error, the return value is unspecified, and a negative value is stored in <code>*errcode</code>.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>, and the length of the specified expression is returned. Note that if the expression is still unfinished, subsequent calls may return larger values.
 */
size_t hilbert_expression_getlength(HilbertExpression * restrict expr, int * restrict errcode);

/**
 * Returns a Hilbert expression as an array of handles.
 * If the module on which the expression is based has been freed, the behaviour is undefined.
 *
 * @param expr Pointer to a Hilbert expression.
 * @param length Pointer to a <code>size_t</code> to convey the number of elements in the returned array.
 * @param errcode Pointer to an integer to convey an error code.
 *
 * @return On error, the return value and <code>*length</code> are unspecified, and a negative value is stored in <code>*errcode</code>,
 * 	which may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_NOMEM</code>:
 * 			There was not enough memory available to perform the request.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>, and a pointer to an array of Hilbert handles representing the expression is returned. The length of the array is stored in <code>*length</code>.
 *
 * @sa #hilbert_harray_free()
 * @sa #hilbert_expression_fromarray()
 */
HilbertHandle * hilbert_expression_toarray(HilbertExpression * restrict expr, size_t * restrict length, int * restrict errcode);

/**
 * Returns the variables in a Hilbert expression in order of left-to-right occurrence in its forward Polish representation.
 * If the module on which the expression is based has been freed, the behaviour is undefined.
 *
 * @param expr Pointer to a Hilbert expression.
 * @param count Pointer to a <code>size_t</code> to store the number of variables.
 * @param errcode Pointer to an integer to convey an error code.
 *
 * @return On error, the return value and <code>*count</code> are unspecified, and a negative value is stored in <code>*errcode</code>,
 * 	which may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_NOMEM</code>:
 * 			There was not enough memory available to perform the request.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>, and a pointer to an array of Hilbert handles representing the occurring variables is returned. The length of the array is stored in <code>*length</code>.
 *
 * @sa #hilbert_harray_free()
 */
HilbertHandle * hilbert_expression_variables(HilbertExpression * restrict expr, size_t * restrict count, int * restrict errcode);

/**
 * Creates a new Hilbert expression by substituting the variables in another expression with finished expressions.
 * If the module on which the expressions are based has been freed, the behaviour is undefined.
 *
 * @param src Pointer to the Hilbert expression on which the substitution is to be performed.
 * @param count The number of variables to be substituted.
 * @param substitutions Pointer to an array of <code>count</code> pointers to Hilbert expressions which are to be substituted for the variables of <code>src</code> in left-to-right order in forward Polish representation.
 * 	If <code>count</code> is zero, this may be <code>NULL</code>.
 * @param errcode Pointer to an integer to convey an error code.
 *
 * @return On error, the return value is unspecified, and a negative value is stored in <code>*errcode</code>,
 * 	which may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_NOMEM</code>:
 * 			There was not enough memory available to perform the request.
 * 		- <code>#HILBERT_ERR_COUNT_MISMATCH</code>:
 * 			<code>count</code> does not equal the number of variables in <code>src</code>.
 * 		- <code>#HILBERT_ERR_INVALID_MODULE</code>:
 * 			One of the expressions in <code>substitutions</code> is not based on the same module as <code>src</code>.
 * 		- <code>#HILBERT_ERR_INVALID_EXPR</code>:
 * 			One of the expressions in <code>substitutions</code> is unfinished.
 * 		- <code>#HILBERT_ERR_NO_EQUIVALENCE</code>:
 * 			The kind of one of the expressions in <code>substitutions</code> is not equivalent to the kind of the variable it is to be substituted for.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>, and a pointer to a new expression with the specified substitutions is returned.
 *
 * @sa #hilbert_expression_free()
 */
HilbertExpression * hilbert_expression_substitute(HilbertExpression * restrict src, size_t count, HilbertExpression ** restrict substitutions, int * restrict errcode);

/**
 * Frees a Hilbert expression.
 *
 * This must be the last function called on an expression pointer.
 * It must be called after all other functions called on the expression pointer have returned.
 * Otherwise, the behaviour is undefined.
 * It is recommended that expressions be freed <em>before</em> the module they are based on is freed
 * because the behaviour of many functions involving expressions is undefined if the underlying module has already been freed.
 *
 * @param expr Pointer to a previously created Hilbert expression.
 */
void hilbert_expression_free(HilbertExpression * expr);

/**
 * Parameterises a Hilbert interface module with another Hilbert interface module.
 *
 * @param dest Pointer to a Hilbert module.
 * @param src Pointer to a Hilbert module different from the module pointed to by <code>dest</code>.
 * @param argc Number of parameter arguments. Must match the number of parameters of <code>src</code>.
 * @param argv Pointer to an array of parameter handles serving as arguments to <code>src</code>.
 * 	If <code>argc == 0</code>, this may be <code>NULL</code>.
 * @param mapper User-provided callback function mapping objects coming from the parameters of <code>src</code> to the argument objects.
 * 	The callback is only called for kinds and functors external to <code>src</code>.
 * 	If <code>argc == 0</code>, this may be <code>NULL</code>.
 * @param userdata Pointer to user-defined data. It is passed as an argument to the userdata parameter of <code>mapper</code>, and is otherwise ignored.
 * @param errcode Pointer to an integer to convey an error code.
 *
 * @return On error, the return value is unspecified,
 * 	and a nonzero value is stored in <code>*errcode</code>. A positive value is a user-defined error code indicating an error which has occurred within <code>mapper</code>.
 * 	Otherwise, the value is negative and may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_NOMEM</code>:
 * 			There was not enough memory available to complete the parameterisation.
 * 		- <code>#HILBERT_ERR_IMMUTABLE</code>:
 * 			The module pointed to by <code>dest</code> is immutable,
 * 			or the module pointed to by <code>src</code> is not immutable.
 * 		- <code>#HILBERT_ERR_INVALID_MODULE</code>:
 * 			One of the modules pointed to by <code>dest</code> or <code>src</code> is not an interface module.
 * 		- <code>#HILBERT_ERR_COUNT_MISMATCH</code>:
 * 			The number of arguments <code>argc</code> does not match the number of parameters of <code>src</code>.
 * 		- <code>#HILBERT_ERR_INVALID_HANDLE</code>:
 * 			One of the handles in the array pointed to by <code>argv</code> is not a valid parameter handle.
 * 		- <code>#HILBERT_ERR_INVALID_MAPPING</code>:
 * 			The <code>mapper</code> callback function returned an invalid object handle.
 * 		- <code>#HILBERT_ERR_MAPPING_CLASH</code>:
 * 			The <code>mapper</code> callback function returned the same object handle for different source objects.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>, and a handle for the new parameter is returned.
 */
HilbertHandle hilbert_module_param(HilbertModule * restrict dest, HilbertModule * restrict src, size_t argc, const HilbertHandle * restrict argv, HilbertMapperCallback mapper, void * userdata, int * restrict errcode);

/**
 * Imports a Hilbert interface module into a Hilbert proof module.
 *
 * @param dest Pointer to a Hilbert proof module.
 * @param src Pointer to a Hilbert interface module.
 * @param argc Number of parameter arguments. Must match the number of parameters of the module pointed to by <code>src</code>.
 * @param argv Pointer to an array of parameter handles serving as arguments to the module pointed to by <code>src</code>.
 * 	If <code>argc == 0</code>, this may be <code>NULL</code>.
 * @param mapper User-provided callback function mapping objects coming from the parameters of the module pointed to by <code>src</code> to the argument objects.
 * 	The callback is only called for kinds, functors and statements <!--FIXME--> external to the module pointed to by <code>src</code>.
 * 	If <code>argc == 0</code>, this may be <code>NULL</code>.
 * @param userdata Pointer to user-defined data. It is passed as an argument to the userdata parameter of <code>mapper</code>, and is otherwise ignored.
 * @param errcode Pointer to an integer to convey an error code.
 *
 * @return On error, the return value is unspecified
 * 	and a nonzero value is stored in <code>*errcode</code>. A positive value is a user-defined error code indicating an error which has occurred within <code>mapper</code>.
 * 	Otherwise, the value is negative and may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_NOMEM</code>:
 * 			There was not enough memory available to complete the import.
 * 		- <code>#HILBERT_ERR_IMMUTABLE</code>:
 * 			The module pointed to by <code>src</code> is not immutable.
 * 		- <code>#HILBERT_ERR_INVALID_MODULE</code>:
 * 			The module pointed to by <code>dest</code> is not a proof module,
 * 			or the module pointed to by <code>src</code> is not an interface module.
 * 		- <code>#HILBERT_ERR_COUNT_MISMATCH</code>:
 * 			The number of arguments <code>argc</code> does not match the number of parameters of the module pointed to by <code>src</code>.
 * 		- <code>#HILBERT_ERR_INVALID_HANDLE</code>:
 * 			One of the handles in the array pointed to by <code>argv</code> is not a valid parameter handle.
 * 		- <code>#HILBERT_ERR_INVALID_MAPPING</code>:
 * 			The <code>mapper</code> callback function returned an invalid object handle.
 * 		- <code>#HILBERT_ERR_MAPPING_CLASH</code>:
 * 			The <code>mapper</code> callback function returned the same object handle for different source objects.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>, and a handle for the new parameter is returned.
 */
HilbertHandle hilbert_module_import(HilbertModule * restrict dest, HilbertModule * restrict src, size_t argc, const HilbertHandle * restrict argv, HilbertMapperCallback mapper, void * userdata, int * restrict errcode);

/**
 * Exports a Hilbert interface module from a Hilbert proof module.
 *
 * @param dest Pointer to a Hilbert proof module.
 * @param src Pointer to a Hilbert interface module.
 * @param argc Number of parameter arguments.
 * 	Must match the number of parameters of the module pointed to by <code>src</code>.
 * @param argv Pointer to an array of parameter handles serving as arguments to the module pointed to by <code>src</code>.
 * 	If <code>argc == 0</code>, this may be <code>NULL</code>.
 * @param mapper User-provided callback function mapping objects from the module pointed to by <code>src</code> to the
 * 	objects in the module pointed to by <code>dest</code>.
 * 	The callback is called for kinds, functors and statements <!--FIXME-->.
 * 	If <code>argc == 0</code>, this may be <code>NULL</code>.
 * @param userdata Pointer to user-defined data.
 * 	It is passed as an argument to the userdata parameter of <code>mapper</code>, and is otherwise ignored.
 * @param errcode Pointer to an integer to convey an error code.
 *
 * @return On error, the return value is unspecified and a nonzero value is stored in <code>*errcode</code>.
 * 	A positive value is a user-defined error code indicating an error which has occurred within <code>mapper</code>.
 * 	Otherwise, the value is negative and may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_NOMEM</code>:
 * 			There was not enough memory available to complete the export.
 * 		- <code>#HILBERT_ERR_IMMUTABLE</code>:
 * 			The module pointed to by <code>src</code> is not immutable.
 * 		- <code>#HILBERT_ERR_INVALID_MODULE</code>:
 * 			The module pointed to by <code>dest</code> is not a proof module,
 * 			or the module pointed to by <code>src</code> is not an interface module.
 * 		- <code>#HILBERT_ERR_COUNT_MISMATCH</code>:
 * 			The number of arguments <code>argc</code> does not match the number of parameters of
 * 			the module pointed to by <code>src</code>.
 * 		- <code>#HILBERT_ERR_INVALID_HANDLE</code>:
 * 			One of the handles in the array pointed to by <code>argv</code> is not a valid parameter handle.
 * 		- <code>#HILBERT_ERR_INVALID_MAPPING</code>:
 * 			The <code>mapper</code> callback function returned an invalid object handle.
 * 		- <code>#HILBERT_ERR_MAPPING_CLASH</code>:
 * 			The <code>mapper</code> callback function returned the same object handle
 * 			for different source objects.
 * 		- <code>#HILBERT_ERR_NO_EQUIVALENCE</code>:
 * 			The <code>mapper</code> callback function mapped two equivalent kinds from the module pointed
 * 			to by <code>src</code> to two inequivalent kinds in the module pointed to by <code>dest</code>.
 *
 * 	On success, <code>0</code> is stored in <code>*errcode</code>, and a handle for the new parameter is returned.
 */
HilbertHandle hilbert_module_export(HilbertModule * restrict dest, HilbertModule * restrict src, size_t argc,
		const HilbertHandle * restrict argv, HilbertMapperCallback mapper, void * userdata,
		int * restrict errcode);

/**
 * Returns all objects of a Hilbert module.
 *
 * @param module Pointer to a Hilbert module.
 * @param size Pointer to a location where the size of the returned array can be stored.
 * @param errcode Pointer to a location where an integer error code can be stored.
 *
 * @return On error, a negative value is stored in <code>*errcode</code> and <code>*size</code> and
 * 	the return value is unspecified. <code>*errcode</code> may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_NOMEM</code>:
 * 			There was not enough memory available to perform the request.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>,
 * 	and a pointer to an array of Hilbert handles whose size is stored in <code>*size</code> is returned.
 * 	The handles in the returned array correspond to the Hilbert objects in the module pointed to by
 * 	<code>module</code>. The handles are stored in the order in which they were created. If several handles were
 * 	created simultaneously in an atomic operation (such as a parameterisation), they adhere to the order with
 * 	respect to the other handles, but among themselves, the order is unspecified.
 * 	The returned array only corresponds to the current state of the module.
 * 	It can be freed with <code>#hilbert_harray_free()</code>.
 *
 * @sa #hilbert_harray_free()
 */
HilbertHandle * hilbert_module_getobjects(HilbertModule * restrict module, size_t * restrict size, int * restrict errcode);

/**
 * Returns the type flags of the object with the specified handle.
 *
 * @param module Pointer to a Hilbert module.
 * @param object Object handle of an object in <code>module</code>.
 * @param errcode Pointer to a location where an integer error code can be stored.
 *
 * @return On error, a negative value is stored in <code>*errcode</code> and the return value is unspecified.
 * 	<code>*errcode</code> may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_INVALID_HANDLE</code>:
 * 			<code>object</code> is not a valid object handle for <code>module</code>.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>,
 * 	and a bitwise OR of one or more of the following flags is returned:
 * 		- <code>#HILBERT_TYPE_KIND</code>:
 * 			The object specified by <code>object</code> is a kind.
 * 		- <code>#HILBERT_TYPE_VKIND</code>:
 * 			The kind specified by <code>object</code> is a variable kind.
 * 		- <code>#HILBERT_TYPE_PARAM</code>:
 * 			The object specified by <code>object</code> is a parameter.
 * 		- <code>#HILBERT_TYPE_EXTERNAL</code>:
 * 			The object specified by <code>object</code> was created through a parameterisation, an import,
 * 			or an export.
 */
unsigned int hilbert_object_gettype(HilbertModule * restrict module, HilbertHandle object, int * restrict errcode);

/**
 * Returns the parameter through which an object was created.
 *
 * @param module Pointer to a Hilbert module.
 * @param object Object handle of an object in <code>module</code>.
 * @param errcode Pointer to a location where an integer error code can be stored.
 *
 * @return On error, a negative value is stored in <code>*errcode</code> and the return value is unspecified.
 * 	<code>*errcode</code> may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_INVALID_HANDLE</code>:
 * 			<code>object</code> is not a valid object handle for the module pointed to by <code>module</code>,
 * 			or the object is not an external object.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>,
 * 	and the parameter handle through which the object corresponding to <code>object</code> was created is returned.
 */
HilbertHandle hilbert_object_getparam(HilbertModule * restrict module, HilbertHandle object, int * restrict errcode);

/**
 * Returns a pointer to the source module from the parameterisation through which an object was created.
 * If the source module has already been freed when this function is called, the behaviour is undefined.
 *
 * @param module Pointer to a Hilbert module.
 * @param object Object handle of an object in the module pointed to by <code>module</code>.
 * @param errcode Pointer to a location where an integer error code can be stored.
 *
 * @return On error, a negative value is stored in <code>*errcode</code> and the return value is unspecified.
 * 	<code>*errcode</code> may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_INVALID_HANDLE</code>:
 * 			<code>object</code> is not a valid object handle for the module pointed to by <code>module</code>,
 * 			or the object is not an external object.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>,
 * 	and a pointer to the source module of the object is returned.
 */
HilbertModule * hilbert_object_getsource(HilbertModule * restrict module, HilbertHandle object, int * restrict errcode);

/**
 * Returns the source module handle of an object.
 * If the source module has already been freed when this function is called, the behaviour is undefined.
 *
 * @param module Pointer to a Hilbert module.
 * @param object Object handle of an object in the module pointed to by <code>module</code>.
 * @param errcode Pointer to a location where an integer error code can be stored.
 *
 * @return On error, a negative value is stored in <code>*errcode</code> and the return value is unspecified.
 * 	<code>*errcode</code> may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_INVALID_HANDLE</code>:
 * 			<code>object</code> is not a valid object handle for the module pointed to by <code>module</code>,
 * 			or the object is not an external object.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>,
 * 	and the handle in the source module is returned.
 *
 * @sa #hilbert_object_getsource()
 */
HilbertHandle hilbert_object_getsourcehandle(HilbertModule * restrict module, HilbertHandle object,
		int * restrict errcode);

/**
 * Returns the destination module handle of an object.
 * If the source module has already been freed when this function is called, the behaviour is undefined.
 *
 * @param module Pointer to the destination Hilbert module.
 * @param param Parameter handle in the destination module.
 * @param object Object handle of an object in the source module.
 * @param errcode Pointer to a location where an integer error code can be stored.
 *
 * @return On error, a negative value is stored in <code>*errcode</code> and the return value is unspecified.
 * 	<code>*errcode</code> may be one of the following error codes:
 * 		- <code>#HILBERT_ERR_INVALID_HANDLE</code>:
 * 			<code>param</code> is not a valid parameter handle in the destination module,
 * 			<code>object</code> is not a valid object handle in the source module,
 * 			or the object has no destination in the module pointed to by <code>module</code>.
 * 	On success, <code>0</code> is stored in <code>*errcode</code>,
 * 	and the handle in the destination module is returned.
 */
HilbertHandle hilbert_object_getdesthandle(HilbertModule * restrict module, HilbertHandle param, HilbertHandle object,
		int * restrict errcode);

#endif
