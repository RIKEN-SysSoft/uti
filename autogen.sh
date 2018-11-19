#!/usr/bin/bash


# Prevent the following issues associated with recursive autoreconf
# (1) top_srcdir in am__aclocal_m4_deps in contrib/hwloc/Makefile.in is broken
# (2) AC_SUBST([MAKE]) is missing in contrib/hwloc/configure.ac.
#     Do the following if you want to fix it manually.
#     if ! grep -qE 'AC_CHECK_PROGS\(MAKE,make gnumake nmake pmake smake\)' contrib/hwloc/configure.ac; then
#         sed -i '/^AM_INIT_AUTOMAKE/ a if test "X$MAKE" = "X" ; then\n\tAC_CHECK_PROGS(MAKE,make gnumake nmake pmake smake)\nfi' contrib/hwloc/configure.ac
#     fi
subdirs=". contrib/hwloc"
for subdir in $subdirs; do
    (cd $subdir && autoreconf -iv --no-recursive)
done
