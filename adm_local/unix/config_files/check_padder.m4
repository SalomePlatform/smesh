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

AC_DEFUN([CHECK_PADDER],[
AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_PROG_CPP])dnl
AC_REQUIRE([CHECK_CGAL])

AC_CHECKING(for PADDER)

padder_ok=no

AC_ARG_WITH(padder,
    [  --with-padder=DIR                 root directory path to PADDER installation ],
    [PADDERHOME="$withval"
      AC_MSG_RESULT("select $withval as path to PADDER")
    ])

if test "x$PADDERHOME" != "xno"; then
    if test "x$PADDERHOME" == "xyes"; then
        PADDERHOME=""
    fi
    if test -z $PADDERHOME
    then
        AC_MSG_WARN(undefined PADDERHOME variable which specify PADDER installation directory)
        AC_PATH_PROG(BINDIR, padder.exe)
        if test "x$BINDIR" != "x" ; then
            PADDERHOME=$BINDIR
            PADDERHOME=`echo ${PADDERHOME} | sed -e "s,[[^/]]*$,,;s,/$,,;s,^$,.,"`
            PADDERHOME=`echo ${PADDERHOME} | sed -e "s,[[^/]]*$,,;s,/$,,;s,^$,.,"`
        fi
    fi
    if test ! -z $PADDERHOME
    then
        AC_CHECK_FILE($PADDERHOME/padder.env,padder_ok=yes,padder_ok=no)
    fi
fi

if  test "x$padder_ok" = "xyes"
then
  CPPFLAGS="-DWITH_PADDER $CPPFLAGS"
fi

AC_SUBST(PADDERHOME)

AC_MSG_RESULT(for PADDER: $padder_ok)

AM_CONDITIONAL(WITH_PADDER, [test x"$padder_ok" = xyes])

])dnl
