AC_INIT(AATM,
	0.4.2,
	b.nikolic@mrao.cam.ac.uk)

dnl Library versioning (current:revision:age)
dnl See the libtool manual for an explanation of the numbers
dnl
dnl aatm-0.2 libatm 0:0:0
dnl aatm-0.3 libatm 1:0:0
dnl aatm-0.4 libatm 2:0:0
dnl aatm-0.4.1 libatm 2:1:0
dnl aatm-0.4.2 libatm 2:2:0

AATM_LT_VERSION="2:2:0"
AC_SUBST(AATM_LT_VERSION)


AC_CONFIG_AUX_DIR(config)

AM_INIT_AUTOMAKE()

AC_CONFIG_HEADERS(config.h)

AC_LIBTOOL_DLOPEN
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


AC_PROG_CC
AC_PROG_CXX

AX_BOOST_BASE(1.34.0)
AX_BOOST_PROGRAM_OPTIONS

AM_CONDITIONAL([TESTDIR_OPT], 
	       [test "$ax_cv_boost_program_options" == yes])


AC_OUTPUT(Makefile 
          src/Makefile
	  test/Makefile
	  otest/Makefile
	  aatm.pc
	  doc/developer.doxy
	  )
	 
