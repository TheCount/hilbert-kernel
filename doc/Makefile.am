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

DX_DOCFILES = @top_srcdir@/src/hilbert.h

include aminclude.am

if DX_COND_doc
DX_CLEANFILES += texput.log @DX_DOCDIR@
MOSTLYCLEANFILES = $(DX_CLEANFILES)

all-local: doxygen-doc

endif DX_COND_doc

