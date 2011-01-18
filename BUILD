These are installation instructions for the Hilbert kernel library.

Please remember that the library is not yet finished.


Downloading the source
======================

If not already present, install git (from http://git-scm.com/) on your system.
Then issue

$ git clone git://github.com/TheCount/hilbert-kernel.git /path/to/source/dir

This should create a clone of the hilbert-kernel source code repository at
/path/to/source/dir on your system.


Building the source
===================

The Hilbert kernel library should be buildable with any ISO C99 compatible
compiler (at least in its single-threaded incarnation). As a convenience,
build files for use with the GNU build system (autotools) are provided. If you
want to build with autotools, first install autoconf, automake and libtool (for
example, via your system's package manage). Then issue the following commands from
the source root directory:

$ autoreconf -i
$ ./configure
$ make
$ make check


Installing the library
======================

Become root and issue

$ make install

This will install the files under /usr/local on your system. If you prefer a
different location, you may pass the --prefix parameter to the configure step
in the "Building the source" step. Run ./configure --help for more
information.

