include_HEADERS += src/include/uti.h

AM_CPPFLAGS += \
	-I$(top_srcdir)/src/include \
	-I$(top_builddir)/src/include

include $(top_srcdir)/src/linux/Makefile.mk
include $(top_srcdir)/src/mckernel/Makefile.mk
