#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
m4_include([../maint/version.m4])
AC_INIT([UTI], 
	UTI_VERSION_m4,
	[masamichi.takagi@riken.jp],
	[uti],
	[http://])

if test "x$prefix" != "xNONE" && test -d "$prefix"; then
   if test "x`(cd \"$prefix\"; echo \"$PWD\")`" = "x`(cd \"$srcdir\"; echo \"$PWD\")`" || \
   test "x`(cd \"$prefix\"; echo \"$PWD\")`" = "x$PWD" ; then
   AC_MSG_ERROR([The install directory (--prefix=) cannot be the same as the build or source directory.])
   fi
fi

CONFIGURE_ARGS_CLEAN=`echo $* | tr '"' ' '`
AC_SUBST(CONFIGURE_ARGS_CLEAN)

# print out the configure options
CONFIGURE_ARGUMENTS="$ac_configure_args"
AC_SUBST(CONFIGURE_ARGUMENTS)
if test -n "$ac_configure_args" ; then
    echo "Configuring uti/test with $ac_configure_args"
else
    echo "Configuring uti/test"
fi

# config.h location and templates
AC_CONFIG_HEADER([src/include/config.h])

AH_TOP([/* 
 * \file config.h
 *  License details are found in the file LICENSE.
 * \author Masamichi Takagi  <masamichi.takagi@riken.jp> \par
 * 	Copyright (C) 2017-2018  Masamichi Takagi
 */
#ifndef _CONFIG_H_INCLUDED_
#define _CONFIG_H_INCLUDED_
])

AH_BOTTOM([
#endif /* !defined(_CONFIG_H_INCLUDED_) */
])

dnl Set the directory that contains support scripts such as install-sh and
dnl config.guess
AC_CONFIG_AUX_DIR(confdb)
dnl our macro dir is the same as our aux dir
AC_CONFIG_MACRO_DIR([confdb])

# Save a copy of precious flags as USER_* before any of these flags
# are being modified by configure tests.
PAC_PREFIX_ALL_FLAGS(USER)

# Add UTITEST_* to the appropriate flags
AC_ARG_VAR(UTITEST_CFLAGS,
	[extra CFLAGS used in building UTI libraries])
AC_ARG_VAR(UTITEST_CPPFLAGS,
	[extra CPPFLAGS used in building UTI libraries])
AC_ARG_VAR(UTITEST_LDFLAGS,
	[extra LDFLAGS used in building UTI libraries])
AC_ARG_VAR(UTI_LIBS,
	[extra LIBS used in building UTI libraries])
CFLAGS="$CFLAGS $UTITEST_CFLAGS"
CPPFLAGS="$CPPFLAGS $UTITEST_CPPFLAGS -I${prefix}/include"
LDFLAGS="$LDFLAGS $UTITEST_LDFLAGS -L${prefix}/lib"
LIBS="$LIBS $UTITEST_LIBS"

dnl -----
dnl setup top-level argument handling
AC_ARG_ENABLE(g,
[  --enable-g=option - Control the debugging level. Option is a list of
                      comma separated names including
        none     - No debug compiler flag is appended.
        dbg      - Append -g to compiler flags, which are used
                   for compiling the programs.
],,enable_g=none)

AC_ARG_ENABLE(fast,
[  --enable-fast=option - Control the optimization level. Option is a 
                         list of comma separated names including
        none     - No optimization flag is appended.
        O<n>     - Append -O<n> to compiler flags, whcih are used 
                   for compiling the programs.
],,enable_fast=none)

# Define with_rm
AC_ARG_WITH(rm,
	AC_HELP_STRING([--with-rm], [Specify resource manager used, e.g. linux, mckernel, mos, etc.]),,
	with_rm=linux)

# Export with_rm to Makefile.am
AC_SUBST(with_rm)

# Export with_rm to subconfigure.m4
export with_rm

# Add hwloc to RPATH because -lhwloc is inherited from parent configure.ac
PAC_APPEND_FLAG([-Wl,-rpath -Wl,${with_hwloc}/lib],[LDFLAGS])

# Find a C compiler.
PAC_PROG_CC
AC_PROG_CC_C_O dnl needed for automake "silent-rules"
PAC_PUSH_FLAG([CFLAGS])
AC_PROG_CPP
# Bug in autoconf. Restore cross settings
if test "$pac_cross_compiling" = "yes" -a "$ac_cv_prog_cc_cross" = "no" ;  then
    AC_MSG_RESULT([Resetting cross compilation to yes])
    cross_compiling=yes
    ac_cv_prog_cc_cross=yes
fi
PAC_POP_FLAG([CFLAGS])

dnl now that autoconf and core compilers are setup, init automake and libtool
AM_INIT_AUTOMAKE([-Wall -Wno-portability-recursive foreign 1.12.3 silent-rules subdir-objects])
AM_MAINTAINER_MODE([enable])

AM_PROG_AR

# prevent libtool from creating shared library
#LT_INIT([disable-shared])
LT_INIT()

# make "make" silent
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# compute canonical build/host system type variable
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

# enable better caching control
PAC_ARG_CACHING

# Set CFLAGS such as "-Wstrict-prototypes". Do this *first* because
# it may influence the output of the other tests
PAC_ARG_STRICT

# This test is complicated by the fact that top_srcdir is not set until
# the very end of configure. Instead, we get it ourselves.
if test -z "$top_srcdir" ;  then
    use_top_srcdir=$srcdir
else
    use_top_srcdir=$top_srcdir
fi
if test -z "$master_top_srcdir" ; then
    # This needs to be an absolute pathname
    case "$use_top_srcdir" in
    /*) ;;
    *)
        use_top_srcdir=`(cd $use_top_srcdir && pwd)`
	;;
    esac
    master_top_srcdir=$use_top_srcdir
fi
# Get the directory that we're running in...
if test -z "$master_top_builddir" ; then
    master_top_builddir="`pwd`"
fi
AC_SUBST(master_top_builddir)
AC_SUBST(master_top_srcdir)
export master_top_builddir
export master_top_srcdir

# enable-fast
save_IFS="$IFS"
IFS=","
for option in $enable_fast ; do
    case "$option" in
    	 O*)
         enable_default_optimize=yes
    	 UTI_DEFAULT_COPTS="-$option"
	 ;;
         none)
	 ;;
	 *)
	 IFS="$save_IFS"
	 AC_MSG_WARN([unknown value $option for --enable-fast])
	 IFS=","
	 ;;
    esac
done
IFS="$save_IFS"

if test "$enable_default_optimize" = "yes" \
   -a -n "$UTI_DEFAULT_COPTS" ; then
   CFLAGS="$CFLAGS $UTI_DEFAULT_COPTS"
fi
AC_MSG_RESULT([CFLAGS is set to $CFLAGS])

# Set up cross compile options
pac_cross_compiling=no
if test "$with_cross" != "no" ; then
    if test -s "$with_cross" ; then
        AC_MSG_RESULT([Reading values from cross-compilation file $with_cross])
        . $with_cross
        # Autoconf 2.52 no longer sets cross_compiling except with the
        # awkward "targethost" options.
        pac_cross_compiling=yes
        cross_compiling=yes
        ac_cv_prog_cc_cross=yes
        export cross_compiling
        # Export all cross variables. Any subsidiary configure should also
        # export CROSS_xxx
        rm -f confcross
        (set) 2>&1 | grep CROSS_ | \
            sed -e 's/^/export /g' -e 's/=.*//g' > confcross
        . confcross
        rm -f confcross
    fi
fi

# enable-g
save_IFS="$IFS"
IFS=","
for option in $enable_g ; do
    case "$option" in
    	 debug|dbg)
    	 enable_append_g=yes
    	 ;;
	 none)
	 ;;
    	 *)
    	 IFS=$save_IFS
    	 AC_MSG_WARN([unknown value $option for enable-g])
    	 IFS=","
    	 ;;
    esac
done
IFS="$save_IFS"

if test "$enable_append_g" = "yes" ; then
   CFLAGS="$CFLAGS -g"
   AC_MSG_RESULT([CFLAGS is set to $CFLAGS])
fi

# determine rpath and other shared library flags for CC
# src/env may not exist yet in a vpath build
$MKDIR_P src/env
cc_shlib_conf=src/env/cc_shlib.conf
PAC_COMPILER_SHLIB_FLAGS([CC],[$cc_shlib_conf])
AC_SUBST_FILE([cc_shlib_conf])

# do compilation tests using CC and CPP
AC_LANG_C

# look for pthread
AC_CHECK_LIB([pthread], [pthread_create], [PAC_PREPEND_FLAG([-lpthread],[LIBS])], [AC_MSG_ERROR([could not find pthread library.  Configure aborted])])

# look for ar
AC_CHECK_PROGS(AR,ar)
if test -z "$AR" ; then
    AC_MSG_ERROR([The program ar is required])
fi
AC_ARG_VAR(AR_FLAGS,[AR command flags]) # causes AC_SUBST too

# note that we set RANLIB to true if we don't find it (some systems neither
# need it nor provide it)
AC_CHECK_PROGS(RANLIB,ranlib,true)

# set variable INSTALL to "install" program
AC_PROG_INSTALL
PAC_PROG_CHECK_INSTALL_WORKS

#we also need mkdir -p.
PAC_PROG_MKDIR_P
PAC_PROG_MAKE

# check for bash to allow more robust shell scripts
AC_PATH_PROG(BASH_SHELL,bash)

# check for C compiler characteristics
AC_C_CONST
AC_C_VOLATILE
AC_C_RESTRICT
AC_C_INLINE

# update the cache first with the results of the previous configure steps
PAC_CREATE_BASE_CACHE

# export uti_{src,build}dir to Makefile.am
AC_SUBST(uti_srcdir)
AC_SUBST(uti_builddir)

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h unistd.h])

# define size_t to a suitable type if standard headers do not define it
AC_TYPE_SIZE_T

dnl if we rerun configure, place a file in the lib directory with the
dnl date. we can use this to avoid rebuilding the library when
dnl a build aborts due to an error (this is intended to help developers)
AC_OUTPUT_COMMANDS([if [ ! -d lib ] ;  then mkdir lib; fi
date > lib/newconfig])

#################################################################################
# cause libtool script to be built now so that we can use it to test one last
# linking issue on Darwin
LT_OUTPUT

AC_CONFIG_FILES([Makefile
                 ])
AC_OUTPUT
