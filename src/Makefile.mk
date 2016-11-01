AM_CPPFLAGS += -I$(top_srcdir)/src/include \
               -I$(top_builddir)/src/include

lib_lib@UTILIBNAME@_la_SOURCES += \
	src/uti.c

#noinst_HEADERS += \
#	src/uti/include/uti.h

