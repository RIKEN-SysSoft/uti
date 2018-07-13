AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I confdb

prefix=@prefix@

# automake requires that we initialize variables to something, even just empty,
# before appending to them with "+="
AM_CPPFLAGS = 
AM_LDFLAGS = 
bin_PROGRAMS =
DISTCLEANFILES =
EXTRA_DIST =
noinst_HEADERS = include/util.h
nodist_noinst_HEADERS = include/config.h

# Define targets
bin_PROGRAMS += create

# Define target-global CPPFLAGS, LDFLAGS
AM_CPPFLAGS += \
	-I$(prefix)/include

AM_LDFLAGS += \
	-L$(prefix)/lib -luti

# Define target-specific SOURCES, CPPFLAGS, LDFLAGS
create_SOURCES = create.c

# Specify subdirectories to build
DIST_SUBDIRS = ${external_subdirs}
SUBDIRS = ${external_subdirs}

# Build the current directory
SUBDIRS += .
DIST_SUBDIRS += .

# Specify files that must be distributed but not covered automatically
# EXTRA_DIST +=

DISTCLEANFILES += config.system
