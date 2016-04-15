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

AC_DEFUN([CHECK_CGAL],[
AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_PROG_CPP])dnl
AC_REQUIRE([CHECK_BOOST])

AC_CHECKING(for CGAL)

AC_ARG_WITH(cgal,
    [  --with-cgal=DIR                 root directory path to CGAL installation ],
    [CGALHOME="$withval"
      AC_MSG_RESULT("select $withval as path to CGAL")
    ])

AC_SUBST(CGAL_INCLUDES)
AC_SUBST(CGAL_LIBS)
AC_SUBST(CGALHOME)

CGAL_INCLUDES=""
CGAL_LIBS=""

cgal_ok=no

LOCAL_INCLUDES=""
LOCAL_LIBS="-lCGAL"

if test "x$CGALHOME" != "xno"; then
    if test "x$CGALHOME" == "xyes"; then
        CGALHOME=""
    fi
    if test -z $CGALHOME
    then
        AC_MSG_WARN(undefined CGALHOME variable which specify CGAL library installation directory)
        AC_PATH_PROG(BINDIR, cgal_create_cmake_script)
        if test "x$BINDIR" != "x" ; then
            CGALHOME=$BINDIR
            CGALHOME=`echo ${CGALHOME} | sed -e "s,[[^/]]*$,,;s,/$,,;s,^$,.,"`
            CGALHOME=`echo ${CGALHOME} | sed -e "s,[[^/]]*$,,;s,/$,,;s,^$,.,"`
        fi
    fi
    if test ! -z $CGALHOME
    then
        cgal_ok=yes
        LOCAL_INCLUDES="-I$CGALHOME/include"
        if test "x$CGALHOME" != "x/usr"; then
            LOCAL_LIBS="-L$CGALHOME/lib $LOCAL_LIBS"
        fi
    fi

dnl check cgallib header

    # CPPFLAGS_old=$CPPFLAGS
    # CPPFLAGS="$CPPFLAGS $LOCAL_INCLUDES -I$BOOSTDIR/include"

    # AC_CHECK_HEADER(algorithm.h,cgal_ok=yes ,cgal_ok=no)

    # CPPFLAGS=$CPPFLAGS_old
fi

# if  test "x$cgal_ok" = "xyes"
# then
# dnl check cgal library
#   LIBS_old="$LIBS"
#   LIBS="$LIBS $LOCAL_LIBS"
#   AC_CHECK_LIB(cgns,cg_open,cgns_ok=yes,cgns_ok=no)
#   LIBS="$LIBS_old" 
# fi

if  test "x$cgal_ok" = "xyes"
then
  CGAL_LIBS="$LOCAL_LIBS"
  CGAL_INCLUDES="$LOCAL_INCLUDES"
  CPPFLAGS="-DWITH_CGAL $CPPFLAGS"
fi

AC_MSG_RESULT(for CGAL: $cgal_ok)

AM_CONDITIONAL(WITH_CGAL, [test x"$cgal_ok" = xyes])

])dnl
