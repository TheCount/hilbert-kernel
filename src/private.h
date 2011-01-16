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

#include"threads/hthreads.h"

/**
 * Generic object type.
 */
struct Generic {
	unsigned int type;
};

/**
 * Kind.
 */
struct Kind {
	unsigned int type;

	// FIXME: add equivalence classes
};

/**
 * Object.
 */
union Object {
	struct Generic generic;
	struct Kind kind;
};

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
	int immutable;

	/**
	 * Ancillary (user set) data.
	 */
	void * ancillary;
};

#endif
