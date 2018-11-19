#!/usr/bin/bash

# Fix the issue of missing AC_SUBST([MAKE]) in contrib/hwloc/configure.ac
if ! grep -qE 'AC_CHECK_PROGS\(MAKE,make gnumake nmake pmake smake\)' contrib/hwloc/configure.ac; then
    sed -i '/^AM_INIT_AUTOMAKE/ a if test "X$MAKE" = "X" ; then\n\tAC_CHECK_PROGS(MAKE,make gnumake nmake pmake smake)\nfi' contrib/hwloc/configure.ac
fi

# Prevent top_srcdir from getting broken in contrib/hwloc by not using recursive autoreconf
subdirs=". contrib/hwloc"
for subdir in $subdirs; do
    (cd $subdir && autoreconf -iv --no-recursive)
done
