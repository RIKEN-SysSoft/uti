#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
m4_include([maint/version.m4])
AC_INIT([UTI], 
	UTI_VERSION_m4,
	[masamichi.takagi@riken.jp],
	[uti],
	[https://github.com/RIKEN-SysSoft/uti])

if test "x$prefix" != "xNONE" && test -d "$prefix"; then
   if test "x`(cd \"$prefix\"; echo \"$PWD\")`" = "x`(cd \"$srcdir\"; echo \"$PWD\")`" || \
   test "x`(cd \"$prefix\"; echo \"$PWD\")`" = "x$PWD" ; then
   AC_MSG_ERROR([The install directory (--prefix=) cannot be the same as the build or source directory.])
   fi
fi

CONFIGURE_ARGS_CLEAN=`echo $* | tr '"' ' '`
AC_SUBST(CONFIGURE_ARGS_CLEAN)

UTI_VERSION=UTI_VERSION_m4
AC_SUBST([UTI_VERSION])
UTI_RELEASE_DATE="UTI_RELEASE_DATE_m4"
AC_SUBST([UTI_RELEASE_DATE])
libuti_so_version="libuti_so_version_m4"
AC_SUBST([libuti_so_version])

if test -z "$UTI_VERSION" ; then
   AC_MSG_ERROR([UTI_VERSION is empty, check maint/version.m4 for errors])
fi

# print out the configure options
CONFIGURE_ARGUMENTS="$ac_configure_args"
AC_SUBST(CONFIGURE_ARGUMENTS)
if test -n "$ac_configure_args" ; then
    echo "Configuring UTI version $UTI_VERSION with $ac_configure_args"
else
    echo "Configuring UTI version $UTI_VERSION"
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

# Define macros for hwloc macros. Note that it uses definitions done
# by AC_CONFIG_MACRO_DIR
AC_CANONICAL_TARGET
AC_ARG_PROGRAM
AC_USE_SYSTEM_EXTENSIONS

# tell subconfigures that they are built from within UTI
FROM_UTI=yes
export FROM_UTI

# Save a copy of precious flags as USER_* before any of these flags
# are being modified by configure tests.
PAC_PREFIX_ALL_FLAGS(USER)

# Add UTILIB_* to the appropriate flags
AC_ARG_VAR(UTILIB_CFLAGS,
	[extra CFLAGS used in building UTI libraries])
AC_ARG_VAR(UTILIB_CPPFLAGS,
	[extra CPPFLAGS used in building UTI libraries])
AC_ARG_VAR(UTILIB_LDFLAGS,
	[extra LDFLAGS used in building UTI libraries])
AC_ARG_VAR(UTI_LIBS,
	[extra LIBS used in building UTI libraries])
CFLAGS="$CFLAGS $UTILIB_CFLAGS"
CPPFLAGS="$CPPFLAGS $UTILIB_CPPFLAGS"
LDFLAGS="$LDFLAGS $UTILIB_LDFLAGS"
LIBS="$LIBS $UTILIB_LIBS"

# Include subconfigure.m4
m4_include([src/linux/subconfigure.m4])
m4_include([src/mckernel/subconfigure.m4])

# List of directories of subconfigure.m4
m4_define([PAC_SUBCFG_MODULE_LIST],
m4_dquote(
[src_linux],
[src_mckernel]dnl <--- this dnl is important
))

#m4_include([debug.m4])

dnl -----
dnl setup top-level argument handling
AC_ARG_ENABLE(g,
[  --enable-g=option - Control the debugging level. Option is a list of
                      comma separated names including
        none     - No debug compiler flag is appended.
        dbg      - Append -g to compiler flags, which are used
                   for compiling the library. 
],,enable_g=dbg)

AC_ARG_ENABLE(fast,
[  --enable-fast=option - Control the optimization level. Option is a 
                         list of comma separated names including
        O<n>     - Append -O<n> to compiler flags, whcih are used 
                   for compiling the library.
],,enable_fast=defopt)

# Define with_rm
AC_ARG_WITH(rm,
	AC_HELP_STRING([--with-rm], [Specify resource manager used, e.g. linux, mckernel, mos, etc.]),,
	with_rm=linux)

# Export with_rm to Makefile.am
AC_SUBST(with_rm)

# Export with_rm to subconfigure.m4
export with_rm

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

# Don't enable this because it will insert @MAINTAINER_MODE_TRUE@ into
# contrib/hwloc/hwloc/Makefile.in
# AM_MAINTAINER_MODE([enable])

AM_PROG_AR

# prevent libtool from creating shared library
#LT_INIT([disable-shared])
LT_INIT()

# make "make" silent
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

dnl do not attempt to place shell conditionals (either manually or via
dnl AS_IF) around these macros in an effort to save configure time. It will
dnl lead to weird AM_CONDITIONAL erros and potentially other problems.

# compute canonical build/host system type variable
AC_CANONICAL_BUILD
AC_CANONICAL_HOST

# enable better caching control
PAC_ARG_CACHING

# Set CFLAGS such as "-Wstrict-prototypes". Do this *first* because
# it may influence the output of the other tests
PAC_ARG_STRICT

# First check that we have a clean build if we are doing a VPATH build
# PAC_VPATH_CHECK(src/include/uti.h, lib)

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

AC_ARG_VAR([UTILIBNAME],[can be used to override the name of the UTI library (default: "uti")])
UTILIBNAME=${UTILIBNAME:-"uti"}
AC_SUBST(UTILIBNAME)

# Set up default optimization level of compiler
UTI_DEFAULT_COPTS="-O2"

# enable-fast
save_IFS="$IFS"
IFS=","
for option in $enable_fast ; do
    case "$option" in
         defopt)
         enable_default_optimize=yes
    	 UTI_DEFAULT_COPTS="-O0"
	 ;;
    	 O*)
         enable_default_optimize=yes
    	 UTI_DEFAULT_COPTS="-$option"
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

# shm functions may be in librt
AC_SEARCH_LIBS([shm_open], [rt])

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

# Embed hwloc. See "Embedding hwloc in Other Software" web page
AC_MSG_NOTICE([Top level configfure: Embedding contrib/hwloc])

# Include hwloc macros
m4_include([contrib/hwloc/config/hwloc.m4])
m4_include([contrib/hwloc/config/hwloc_check_attributes.m4])
m4_include([contrib/hwloc/config/hwloc_check_visibility.m4])
m4_include([contrib/hwloc/config/hwloc_check_vendor.m4])
m4_include([contrib/hwloc/config/hwloc_components.m4])
m4_include([contrib/hwloc/config/hwloc_pkg.m4])

# Include netloc macros
m4_include([contrib/hwloc/config/netloc.m4])

# Define variables and AC_SUBST them
HWLOC_SET_SYMBOL_PREFIX(uti_)
HWLOC_SETUP_CORE([contrib/hwloc],[have_hwloc=yes],[have_hwloc=no])
HWLOC_SUBDIRS="contrib/hwloc"
AC_SUBST([HWLOC_SUBDIRS])

AS_IF([test "$have_hwloc" == "no"],
[AC_MSG_ERROR([contrib/hwloc is broken])])

# Define conditionals used in contrib/hwloc/Makefile.in
HWLOC_DO_AM_CONDITIONALS
NETLOC_DO_AM_CONDITIONALS

dnl configure any subdirectories. Note that config.status will *not*
dnl reexecute these!
dnl export any important variables first.
dnl invoke the configure scripts in subdirectories directly.
export AR
export AR_FLAGS
export RANLIB
export UTILIBNAME
export CC
export CPPFLAGS
export LIBS
export CFLAGS
export LDFLAGS

# update the cache first with the results of the previous configure steps
PAC_CREATE_BASE_CACHE

# now configure any actual recursively configures subsystems
#for subsys in 

# Get the value of libtool_static_flag
if test ! -z "$UTI_LIBTOOL_STATIC_FLAG" ; then
    uti_libtool_static_flag="$UTI_LIBTOOL_STATIC_FLAG"
else
    uti_libtool_static_flag="-static"
fi
AC_SUBST([uti_libtool_static_flag])

# Export uti_{src,build}dir to test/configure.ac
uti_srcdir=$master_top_srcdir
uti_builddir=$master_top_builddir
export uti_srcdir
export uti_builddir

# ???
if test -z "$includebuild_dir" ; then
    includebuild_dir=`pwd`/src/include
fi
if test -z "$libbuild_dir" ;  then
    libbuild_dir=`pwd`/lib
fi


# the following make these definitions:
# define _UINT<n>_T 1
# if uint<n>_t is available
# if not available, define uint<n>_t as the releate C type, e.g.,
# define uint8_t unsigned char
AC_TYPE_UINT8_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for header files.
AC_CHECK_HEADERS([stdint.h stdlib.h string.h unistd.h])

# define size_t to a suitable type if standard headers do not define it
AC_TYPE_SIZE_T

dnl if we rerun configure, place a file in the lib directory with the
dnl date. we can use this to avoid rebuilding the library when
dnl a build aborts due to an error (this is intended to help developers)
AC_OUTPUT_COMMANDS([if [ ! -d lib ] ;  then mkdir lib; fi
date > lib/newconfig])

# Run PREREQ part of subconfigure.m4
m4_map([PAC_SUBCFG_DO_PREREQ], [PAC_SUBCFG_MODULE_LIST])

# Run BODY part of subconfigure.m4
m4_map([PAC_SUBCFG_CONFIGURE_SUBSYS], [PAC_SUBCFG_MODULE_LIST])

# Configure hwloc with --enable-embedded-mode
export enable_embedded_mode=yes

# Configure test
AC_CONFIG_SUBDIRS([test])

#################################################################################
# cause libtool script to be built now so that we can use it to test one last
# linking issue on Darwin
LT_OUTPUT

AC_CONFIG_FILES([Makefile
                 ])
AC_OUTPUT

