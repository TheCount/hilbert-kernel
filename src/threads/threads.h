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

/* Partial standard C thread emulation through POSIX until C1X is ready */

#ifndef HILBERT_THREADS_THREADS_H__
#define HILBERT_THREADS_THREADS_H__

#include<pthread.h>

/**
 * Mutex type.
 *
 * A variable of this type can hold the identifier for a mutex.
 */
typedef pthread_mutex_t mtx_t;

/**
 * Enumeration constants.
 */
enum {
	/**
	 * Enumeration constant passed to <code>#mtx_init()</code> to create a mutex object
	 * that supports neither timeout nor test and return.
	 */
	mtx_plain = 0x01,

	/**
	 * Enumeration constant passed to <code>#mtx_init()</code> to create a mutex object
	 * that supports recursive locking.
	 */
	mtx_recursive = 0x02,

	/**
	 * Enumeration constant returned by a function to indicate
	 * that the requested operation has succeeded.
	 */
	thrd_success,

	/**
	 * Enumeration constant returned by a function to indicate
	 * that the requested operation has failed.
	 */
	thrd_error,
};

/**
 * Destroys a mutex.
 *
 * Releases any resources used by the mutex pointed to by <code>mtx</code>.
 * No threads can be blocked waiting for the mutex pointed to by <code>mtx</code>.
 *
 * @param mtx Pointer to the mutex to be destroyed.
 */
static inline void mtx_destroy(mtx_t * mtx) {
	pthread_mutex_destroy(mtx);
}

/**
 * Initialises a mutex.
 *
 * Creates a mutex object with properties indicated by <code>type</code>,
 * which, for this very limited implementation, must equal <code>#mtx_plain</code>.
 *
 * @param mtx Pointer to the mutex to be initialised.
 * @param type Type of the mutex.
 *
 * @return On success, <code>#thrd_success</code> is returned,
 * 	and the mutex pointed to by <code>mtx</code> will have been set to a value that
 * 	uniquely identifies the newly created mutex.
 * 	On error, <code>#thrd_error</code> is returned.
 */
static inline int mtx_init(mtx_t * mtx, int type) {
	pthread_mutexattr_t attr;
	int result = thrd_error;
	int rc;

	if (pthread_mutexattr_init(&attr) != 0)
		goto ainitfail;

	if (type == mtx_plain) {
		rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
	} else if (type == (mtx_plain | mtx_recursive)) {
		rc = pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	} else {
		goto wrongtype;
	}
	if (rc != 0)
		goto asetfail;

	if (pthread_mutex_init(mtx, &attr) == 0)
		result = thrd_success;

asetfail:
wrongtype:
	pthread_mutexattr_destroy(&attr);
ainitfail:
	return result;
}

/**
 * Locks a mutex, possibly blocking the current thread.
 *
 * Blocks until it locks the mutex pointed to by <code>mtx</code>.
 * The mutex shall not be locked by the calling thread.
 * Prior calls to this function on the same mutex shall
 * synchronise with this operation.
 *
 * @param mtx Pointer to mutex to be locked.
 *
 * @return On success, <code>#thrd_success</code> is returned.
 * 	On error, <code>#thrd_error</code> is returned.
 */
static inline int mtx_lock(mtx_t * mtx) {
	if (pthread_mutex_lock(mtx) == 0)
		return thrd_success;
	return thrd_error;
}

/**
 * Unlocks a mutex.
 *
 * Unlocks the mutex pointed to by <code>mtx</code>.
 * The mutex pointed to by <code>mtx</code> shall be locked by the calling thread.
 *
 * @param mtx Pointer to mutex to be unlocked.
 *
 * @return On success, <code>#thrd_success</code> is returned.
 * 	On error, <code>#thrd_error</code> is returned.
 */
static inline int mtx_unlock(mtx_t * mtx) {
	if (pthread_mutex_unlock(mtx) == 0)
		return thrd_success;
	return thrd_error;
}

#endif
