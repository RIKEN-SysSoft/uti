[#] start of __file__

# Define PREREQ macro run by configure
AC_DEFUN([PAC_SUBCFG_PREREQ_]PAC_SUBCFG_AUTO_SUFFIX,[
	# Define BUILD_LINUX used in Makefile.mk 
	AM_CONDITIONAL([BUILD_LINUX],[test "X$with_os" = "Xlinux"])
])dnl end AC_DEFUN

# Define BODY macro run by configure
AC_DEFUN([PAC_SUBCFG_BODY_]PAC_SUBCFG_AUTO_SUFFIX,[
# BUILD_LINUX is set above
AM_COND_IF([BUILD_LINUX],[
	AC_MSG_NOTICE([RUNNING CONFIGURE FOR LINUX])

	AC_CHECK_HEADER([sys/capability.h],[libcap_found=yes],[libcap_found=no])
	AS_IF([test "x$libcap_found" != "xyes"],
		[AC_MSG_ERROR([Unable to find sys/capability.h, missing libcap-devel?])])
	AC_CHECK_LIB([cap],[cap_get_proc],[libcap_found=yes],[libcap_found=no])
	AS_IF([test "x$libcap_found" != "xyes"],
		[AC_MSG_ERROR([Unable to find libcap.so, missing libcap-devel?])])

	# Add --with-hwloc=<path> option
	PAC_SET_HEADER_LIB_PATH(hwloc)

        # Check if hwloc is linkable
	PAC_CHECK_HEADER_LIB_FATAL(hwloc, hwloc.h, hwloc, hwloc_topology_init)


])dnl end AM_COND_IF(BUILD_LINUX,...)
])dnl end AC_DEFUN

[#] end of __file__
