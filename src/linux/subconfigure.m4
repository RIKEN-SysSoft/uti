[#] start of __file__

# Define PREREQ macro run by configure
AC_DEFUN([PAC_SUBCFG_PREREQ_]PAC_SUBCFG_AUTO_SUFFIX,[
	# Define BUILD_LINUX used in Makefile.mk 
	AM_CONDITIONAL([BUILD_LINUX],[test "X$with_rm" = "Xlinux"])
])dnl end AC_DEFUN

# Define BODY macro run by configure
AC_DEFUN([PAC_SUBCFG_BODY_]PAC_SUBCFG_AUTO_SUFFIX,[
# BUILD_LINUX is set above
AM_COND_IF([BUILD_LINUX],[

	AC_MSG_NOTICE([RUNNING CONFIGURE FOR LINUX])

	# Append cap path to CPPFLAGS and LDFLAGS via --with-cap=<path>
	PAC_SET_HEADER_LIB_PATH(cap)

	# Export with_cap to Makefile.am
	AS_IF([test -z "$with_cap"],
		[AC_MSG_NOTICE([Link with distro libcap])])
	AC_SUBST(with_cap)

	# Check if cap is linkable
	PAC_CHECK_HEADER_LIB_FATAL(cap, sys/capability.h, cap, cap_get_proc)

])dnl end AM_COND_IF(BUILD_LINUX,...)
])dnl end AC_DEFUN

[#] end of __file__
