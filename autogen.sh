#!/usr/bin/bash

# Fix the issue that contrib/hwloc/configure.ac doesn't have
# AC_PROG_MAKE_SET while contrib/hwloc/Makefile.in has @SET_MAKE@.
if ! grep -q AC_PROG_MAKE_SET contrib/hwloc/configure.ac; then
    sed -i '/AM_INIT_AUTOMAKE/ a AC_PROG_MAKE_SET' contrib/hwloc/configure.ac
fi

autoreconf -iv
