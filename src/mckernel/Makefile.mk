if BUILD_MCKERNEL

AM_CPPFLAGS += \
	-I$(top_srcdir)/src/mckernel/include

lib_lib@UTILIBNAME@_la_SOURCES += \
	src/mckernel/uti.c

endif BUILD_MCKERNEL
