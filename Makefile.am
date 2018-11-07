AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I confdb

# Location of internal hwloc 
HWLOC_SUBDIRS = @HWLOC_SUBDIRS@

# Options used to build hwloc_embedded.la
HWLOC_EMBEDDED_CFLAGS = @HWLOC_EMBEDDED_CFLAGS@
HWLOC_EMBEDDED_CPPFLAGS = @HWLOC_EMBEDDED_CPPFLAGS@

# Location of hwloc_embedded.la
HWLOC_EMBEDDED_LDADD = @HWLOC_EMBEDDED_LDADD@

# Libraries used by hwloc_embedded.la
HWLOC_EMBEDDED_LIBS = @HWLOC_EMBEDDED_LIBS@

# automake requires that we initialize variables to something, even just empty,
# before appending to them with "+="
AM_CFLAGS =
AM_CPPFLAGS = -Wall -I$(top_srcdir)/src/include -I$(top_builddir)/src/include
AM_LDFLAGS =
include_HEADERS = src/include/uti.h
lib_lib@UTILIBNAME@_la_SOURCES = src/common.c
lib_lib@UTILIBNAME@_la_LIBADD = $(HWLOC_EMBEDDED_LDADD)
lib_lib@UTILIBNAME@_la_LDFLAGS = $(HWLOC_EMBEDDED_LIBS) -export-symbols-regex '^uti_'
lib_LTLIBRARIES = lib/lib@UTILIBNAME@.la
nodist_include_HEADERS =
nodist_noinst_HEADERS =
BUILT_SOURCES =
bin_PROGRAMS =
bin_SCRIPTS =
dist_noinst_SCRIPTS =
noinst_DATA =
dist_noinst_DATA =
noinst_HEADERS = src/include/uti_impl.h
noinst_IBRARIES =
sysconf_DATA =
CLEANFILES =
DISTCLEANFILES =
SUFFIXES =
DOC_SUBDIRS =
MANDOC_SUBDIRS =
HTMLDOC_SUBDIRS =
INSTALL_DATA_LOCAL_TARGETS =

# Gather HEADERS, SOURCES, CPPFLAGS
include $(top_srcdir)/src/Makefile.mk

# for $subdir in $SUBDIRS; do make -C $subdir all; done
# Note that hwloc should be built first
SUBDIRS = $(HWLOC_SUBDIRS) . test

# for $subdir in $SUBDIRS; do make -C $subdir distclean; done
DIST_SUBDIRS = $(HWLOC_SUBDIRS) . test

# Specify files to distribute but not covered automatically
EXTRA_DIST = README LICENSE RELEASE CONTRIBUTORS

# Add target name to this variable to add them to the dependencies of the
# clean-local target
CLEAN_LOCAL_TARGETS =

# the configure step checks for the 'lib' dir to decide whether a source 
# directory has been previously used for an in-path build before allowing a
# VPATH build to continue
distclean-local:
	-rm -rf $(top_builddir)/lib
	-rm -rf $(top_builddir)/bin
	-rm -rf $(top_builddir)/include
	-rm -rf $(top_builddir)/share

# created by confdb/aclocal_cache.m4
DISTCLEANFILES += config.system
