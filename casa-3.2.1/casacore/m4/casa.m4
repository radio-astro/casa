#
# Check for external library
# Example: CASA_CHECK_LIBRARY([cfitsio], [-lcfitsio], [[#include<fitsio.h> #include<fitsio2.h>]], CFITSIO_VERSION, 3.14)
#
AC_DEFUN([CASA_CHECK_LIB],
[
        AC_ARG_WITH([$1],
                    [AS_HELP_STRING([--with-$1=prefix],
                        [try this for a non-standard install prefix of the $1 library])],
                    [PATHSET=yes],
                    [PATHSET=no]
        )

        if test x"$PATHSET" = xyes ; then
                AM_CXXFLAGS="$AM_CXXFLAGS -I$with_$1/include"
                AM_LDFLAGS="$AM_LDFLAGS -L$with_$1/lib"
        fi

        OLD_CXXFLAGS=$CXXFLAGS
        OLD_LDFLAGS=$LDFLAGS
        CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
        LDFLAGS="$AM_LDFLAGS $LDFLAGS"

        OLD_LIBS=$LIBS
        LIBS="$LIBS $2"

        AC_MSG_CHECKING([for $1 library headers])
        AC_COMPILE_IFELSE([[
                  $3
                  ]],
                  [AC_MSG_RESULT([yes])
                   AC_MSG_CHECKING([whether $1 library can be linked])
                   AC_LINK_IFELSE([AC_LANG_PROGRAM([], [])],
                                  [AC_MSG_RESULT([yes])
                                   AC_MSG_CHECKING([whether $1 is version $5 or greater])
                                   AC_RUN_IFELSE([
                                   $3
                                   #include <iostream>
                                   int main() {
                                       double required = $5;
                                       double found = $4;
                                       std::cout << "is " << found << "... ";
                                       return !(found >= required);
                                   }
                                   ],
                                    [AC_MSG_RESULT([yes])
                                     FOUND=yes;],
                                    [AC_MSG_RESULT([no])
                                     FOUND=no;]
                                    )],
                                  [AC_MSG_RESULT([no])
                                   LIBS=$OLD_LIBS; dnl reset to old value since $1 was not found
                                   FOUND=no;]
                                 )],
                  [AC_MSG_RESULT([not found])
                   FOUND=no;])

        # reset original *FLAGS
        CXXFLAGS=$OLD_CXXFLAGS
        LDFLAGS=$OLD_LDFLAGS

        # handle check results
        if test x"$FOUND" = xno; then
            AC_MSG_NOTICE([])
            AC_MSG_NOTICE([The $1 library was not found!])
            if test x"$PATHSET" = xyes ; then
                    AC_MSG_NOTICE([ The install prefix '$with_$1' for the $1 library was set. ])
            else
                    AC_MSG_NOTICE([ No non-standard install prefix was set.])
                    AC_MSG_NOTICE([ --> You might want to use '--with-$1=PREFIX'])
            fi
            AC_MSG_NOTICE([])
            AC_MSG_FAILURE([$1 not available -- please check!])
        fi

        # distribute the changed variables among the Makefiles
        AC_SUBST(LIBS)
        AC_SUBST(AM_CXXFLAGS)
        AC_SUBST(AM_LDFLAGS)
])

#
# Same as CASA_CHECK_LIB, but the version to check has type string
# and follows the format  *X.Y.Z*
#
AC_DEFUN([CASA_CHECK_LIB_STR],
[
        AC_ARG_WITH([$1],
                    [AS_HELP_STRING([--with-$1=prefix],
                        [try this for a non-standard install prefix of the $1 library])],
                    [PATHSET=yes],
                    [PATHSET=no]
        )

        if test x"$PATHSET" = xyes ; then
                AM_CXXFLAGS="$AM_CXXFLAGS -I$with_$1/include"
                AM_LDFLAGS="$AM_LDFLAGS -L$with_$1/lib"
        fi

        OLD_CXXFLAGS=$CXXFLAGS
        OLD_LDFLAGS=$LDFLAGS
        CXXFLAGS="$AM_CXXFLAGS $CXXFLAGS"
        LDFLAGS="$AM_LDFLAGS $LDFLAGS"

        OLD_LIBS=$LIBS
        LIBS="$LIBS $2"

        AC_MSG_CHECKING([for $1 library headers])
        AC_COMPILE_IFELSE([[
                  $3
                  ]],
                  [AC_MSG_RESULT([yes])
                   AC_MSG_CHECKING([whether $1 library can be linked])
                   AC_LINK_IFELSE([AC_LANG_PROGRAM([], [])],
                                  [AC_MSG_RESULT([yes])
                                   AC_MSG_CHECKING([whether $1 is version $5 or greater])
                                   AC_RUN_IFELSE([
                                   $3
                                   #include <iostream>
                                   #include <cstring>
                                   int main() {
                                       const char * req = "$5";
                                       char maj = strstr($4, "-")[[1]];
                                       char min = strstr(strstr($4, "-"), ".")[[1]];
                                       char mic = strstr(strstr(strstr($4, "-"), ".")+1, ".")[[1]];
                                       std::cout << "is " << $4 << " " << maj << "." << min << "." << mic << "...";
                                       return !(maj > req[[0]] || (
                                                maj == req[[0]] && (min > req[[2]] ||
                                                             (min == req[[2]] && mic >= req[[4]]))));
                                   }
                                   ],
                                   [AC_MSG_RESULT([yes])
                                     FOUND=yes;],
                                    [AC_MSG_RESULT([no])
                                     FOUND=no;]
                                    )],
                                  [AC_MSG_RESULT([no])
                                   LIBS=$OLD_LIBS; dnl reset to old value since $1 was not found
                                   FOUND=no;]
                                 )],
                  [AC_MSG_RESULT([not found])
                   FOUND=no;])

        # reset original *FLAGS
        CXXFLAGS=$OLD_CXXFLAGS
        LDFLAGS=$OLD_LDFLAGS

        # handle check results
        if test x"$FOUND" = xno; then
            AC_MSG_NOTICE([])
            AC_MSG_NOTICE([The $1 library was not found!])
            if test x"$PATHSET" = xyes ; then
                    AC_MSG_NOTICE([ The install prefix '$with_$1' for the $1 library was set. ])
            else
                    AC_MSG_NOTICE([ No non-standard install prefix was set.])
                    AC_MSG_NOTICE([ --> You might want to use '--with-$1=PREFIX'])
            fi
            AC_MSG_NOTICE([])
            AC_MSG_FAILURE([$1 not available -- please check!])
        fi

        # distribute the changed variables among the Makefiles
        AC_SUBST(LIBS)
        AC_SUBST(AM_CXXFLAGS)
        AC_SUBST(AM_LDFLAGS)
])
