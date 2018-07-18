AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I confdb

# automake requires that we initialize variables to something, even just empty,
# before appending to them with "+="
AM_CPPFLAGS = -Wall 
AM_LDFLAGS = 
include_HEADERS = 
lib_lib@UTILIBNAME@_la_SOURCES =
nodist_include_HEADERS =
nodist_noinst_HEADERS = 
BUILT_SOURCES = 
bin_PROGRAMS =
bin_SCRIPTS = 
dist_noinst_SCRIPTS = 
noinst_DATA =
dist_noinst_DATA = 
noinst_HEADERS = 
noinst_LIBRARIES = 
sysconf_DATA = 
CLEANFILES =
DISTCLEANFILES =
SUFFIXES = 
DOC_SUBDIRS =
MANDOC_SUBDIRS =
HTMLDOC_SUBDIRS =
INSTALL_DATA_LOCAL_TARGETS =

# add target name to this variable to add them to the dependencies of the
# clean-local target
CLEAN_LOCAL_TARGETS =

# gather HEADERS, SOURCES, CPPFLAGS
include $(top_srcdir)/src/Makefile.mk

lib_LTLIBRARIES = lib/lib@UTILIBNAME@.la
lib_lib@UTILIBNAME@_la_LIBADD = $(external_libs)
lib_lib@UTILIBNAME@_la_LDFLAGS = $(external_ldflags) $(ABIVERSIONFLAGS)

# Specify directories to build
SUBDIRS = . test

# Specify directories to distribute
DIST_SUBDIRS = .

# Specify files to distribute but not covered automatically
EXTRA_DIST = README LICENSE RELEASE CONTRIBUTORS

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
