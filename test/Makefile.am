AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I confdb

uti_srcdir=@uti_srcdir@
uti_builddir=@uti_builddir@

# Define distribution targets
noinst_HEADERS = src/include/util.h \
	$(top_builddir)/src/include/config.h

# Define compile options
AM_CPPFLAGS = \
	-Wall \
	-I$(top_srcdir)/src/include \
	-I$(top_builddir)/src/include \
	-I$(uti_srcdir)/src/include
AM_LDFLAGS = \
	-L$(uti_builddir)/lib -Wl,-rpath -Wl,$(uti_builddir)/lib -luti


# Run-target for unit tests (make check)
TESTS = create attr

# Build-target/options for unit tests
check_PROGRAMS = create attr
create_SOURCES = src/create.c
attr_SOURCES = src/attr.c

# Specify directories to build
SUBDIRS = .

# Specify directories to distribute
DIST_SUBDIRS = .

# Specify files to distribute but not covered automatically
EXTRA_DIST =

# created by confdb/aclocal_cache.m4
DISTCLEANFILES = config.system
