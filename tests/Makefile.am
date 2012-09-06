#
#  The Hilbert Kernel Library, a library for verifying formal proofs.
#  Copyright © 2011, 2012 Alexander Klauer
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
#  To contact the author
#     by email: Graf.Zahl@gmx.net
#     on wiki : http://www.wikiproofs.org/w/index.php?title=User_talk:GrafZahl
#

TESTNAMES = module immutable ancillary \
	    kind_create kind_alias kind_id kind_eq vkind_create vkind_alias vkind_id vkind_eq eqc veqc kind_vs_vkind \
	    var_create var_getkind \
	    functor_create functor_getkind functor_getinputkinds \
	    objecttype param import export getobjects object_getparam object_getsource object_getsourcehandle object_getdesthandle \
	    expr_create expr_build expr_array expr_get expr_subexpr expr_vars
check_PROGRAMS = $(TESTNAMES)
AM_CFLAGS = -I../src/
AM_LDFLAGS = -L../src/ -lhilbert
AM_DEFAULT_SOURCE_EXT = .c
TESTS = $(TESTNAMES)
