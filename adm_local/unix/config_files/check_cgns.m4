dnl Copyright (C) 2007-2016  CEA/DEN, EDF R&D, OPEN CASCADE
dnl
dnl Copyright (C) 2003-2007  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
dnl CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS
dnl
dnl This library is free software; you can redistribute it and/or
dnl modify it under the terms of the GNU Lesser General Public
dnl License as published by the Free Software Foundation; either
dnl version 2.1 of the License, or (at your option) any later version.
dnl
dnl This library is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
dnl Lesser General Public License for more details.
dnl
dnl You should have received a copy of the GNU Lesser General Public
dnl License along with this library; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
dnl
dnl See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
dnl

AC_DEFUN([CHECK_CGNS],[
AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_PROG_CPP])dnl
AC_REQUIRE([CHECK_HDF5])dnl

AC_CHECKING(for CGNS)

AC_ARG_WITH(cgns,
    [  --with-cgns=DIR                 root directory path to CGNS installation ],
    [CGNSHOME="$withval"
      AC_MSG_RESULT("select $withval as path to CGNS")
    ])

AC_SUBST(CGNS_INCLUDES)
AC_SUBST(CGNS_LIBS)

CGNS_INCLUDES=""
CGNS_LIBS=""

cgns_ok=no

LOCAL_INCLUDES=""
LOCAL_LIBS="-lcgns $HDF5_LIBS"

if test "x$CGNSHOME" != "xno"; then
    if test "x$CGNSHOME" == "xyes"; then
        CGNSHOME=""
    fi
    if test -z $CGNSHOME
    then
        AC_MSG_WARN(undefined CGNSHOME variable which specify CGNS library installation directory)
        AC_PATH_PROG(BINDIR, cgnsversion)
        if test "x$BINDIR" != "x" ; then
            CGNSHOME=$BINDIR
            CGNSHOME=`echo ${CGNSHOME} | sed -e "s,[[^/]]*$,,;s,/$,,;s,^$,.,"`
            CGNSHOME=`echo ${CGNSHOME} | sed -e "s,[[^/]]*$,,;s,/$,,;s,^$,.,"`
        fi
    fi
    if test ! -z $CGNSHOME
    then
        LOCAL_INCLUDES="-I$CGNSHOME/include"
        if test "x$CGNSHOME" != "x/usr"; then
            LOCAL_LIBS="-L$CGNSHOME/lib $LOCAL_LIBS"
        fi
    fi

dnl check cgnslib header

    CPPFLAGS_old=$CPPFLAGS
    CPPFLAGS="$CPPFLAGS $LOCAL_INCLUDES"

    AC_CHECK_HEADER(cgnslib.h,cgns_ok=yes ,cgns_ok=no)

    CPPFLAGS=$CPPFLAGS_old
fi

if  test "x$cgns_ok" = "xyes"
then

dnl check cgns library

  LIBS_old="$LIBS"
  LIBS="$LIBS $LOCAL_LIBS"
  AC_CHECK_LIB(cgns,cg_open,cgns_ok=yes,cgns_ok=no)

  LIBS="$LIBS_old"
 
fi

if  test "x$cgns_ok" = "xyes"
then
  CGNS_LIBS="$LOCAL_LIBS"
  CGNS_INCLUDES="$LOCAL_INCLUDES"
  CPPFLAGS="-DWITH_CGNS $CPPFLAGS"
fi

AC_MSG_RESULT(for CGNS: $cgns_ok)

AM_CONDITIONAL(WITH_CGNS, [test x"$cgns_ok" = xyes])

])dnl
