AC_INIT(AATM,
	0.06,
	"Bojan Nikolic" <b.nikolic@mrao.cam.ac.uk>)

AC_CONFIG_AUX_DIR(config)

AM_INIT_AUTOMAKE()

AC_CONFIG_HEADERS(config.h)

AC_LIBTOOL_DLOPEN
AC_DISABLE_STATIC
AC_PROG_LIBTOOL


AC_PROG_CC
AC_PROG_CXX

AX_BOOST(1.34.0,
	[echo "GOOD: You have BOOST ... we will be using that!" ],		     
	AC_MSG_ERROR(["ERROR: No BOOST  ... ; you can find it at  http://www.boost.org/ "])  )

AC_OUTPUT(Makefile 
          src/Makefile
	  test/Makefile
	  otest/Makefile
	  aatm.pc
	  doc/developer.doxy
	  )
	 
