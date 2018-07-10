[#] start of __file__

# Define PREREQ macro run by configure
AC_DEFUN([PAC_SUBCFG_PREREQ_]PAC_SUBCFG_AUTO_SUFFIX,[
    # Define BUILD_MCKERNEL used in Makefile.mk 
    AM_CONDITIONAL([BUILD_MCKERNEL],[test "X$with_os" = "Xmckernel"])
])dnl end AC_DEFUN

# Define BODY macro run by configure
AC_DEFUN([PAC_SUBCFG_BODY_]PAC_SUBCFG_AUTO_SUFFIX,[
  # Run selectively because configure runs subconfigures recursively
  AM_COND_IF([BUILD_MCKERNEL],[
    AC_MSG_NOTICE([RUNNING CONFIGURE FOR MCKERNEL])
  ])dnl end AM_COND_IF(BUILD_MCKERNEL,...)
])dnl end AC_DEFUN

[#] end of __file__
