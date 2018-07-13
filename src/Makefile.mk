include_HEADERS += src/include/uti.h
noinst_HEADERS += src/include/uti_impl.h

AM_CPPFLAGS += \
	-I$(top_srcdir)/src/include \
	-I$(top_builddir)/src/include

lib_lib@UTILIBNAME@_la_SOURCES += \
	src/common.c

include $(top_srcdir)/src/linux/Makefile.mk
include $(top_srcdir)/src/mckernel/Makefile.mk
