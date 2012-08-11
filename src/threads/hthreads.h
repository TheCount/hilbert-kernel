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

#ifndef HILBERT_THREADS_HTHREADS_H__
#define HILBERT_THREADS_HTHREADS_H__

#ifndef HILBERT_THREADSAFE

/* No threadsafe library requested.
 * Simply provide dummy implementation which does nothing
 */

/**
 * Dummy mutex declaration macro
 */
#define HILBERT_MUTEX_DECL(x)

/**
 * Dummy enumeration constants.
 */
enum {
	mtx_plain,
	mtx_recursive,
	thrd_success,
	thrd_error
};

/**
 * Dummy mutex destruction.
 *
 * @param mtx Dummy parameter.
 */
#define mtx_destroy(mtx)

/**
 * Dummy mutex init.
 *
 * @param mtx Dummy parameter.
 * @param type Dummy parameter.
 *
 * @return Dummy function always returns <code>#thrd_success</code>.
 */
#define mtx_init(mtx, type) thrd_success

/**
 * Dummy mutex lock.
 *
 * @param mtx Dummy parameter.
 *
 * @return Dummy function always returns <code>#thrd_success</code>.
 */
#define mtx_lock(mtx) thrd_success

/**
 * Dummy mutex unlock.
 *
 * @param mtx Dummy parameter.
 *
 * @return Dummy function always returns <code>#thrd_success</code>.
 */
#define mtx_unlock(mtx) thrd_success

#else /* HILBERT_THREADSAFE is defined */

#define HILBERT_MUTEX_DECL(x) mtx_t x

// #ifdef __STDC_NO_THREADS__ 
#include"threads.h"
// #else
// #include<threads.h>
// #endif

#endif /* !defined HILBERT_THREADSAFE */

#endif
