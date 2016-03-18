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
AC_DEFUN([CHECK_QWT],[
AC_REQUIRE([CHECK_QT])dnl
AC_REQUIRE([AC_LINKER_OPTIONS])dnl

AC_CHECKING(for qwt)

AC_LANG_SAVE
AC_LANG_CPLUSPLUS

qwt_ok=yes

dnl where is qwt ?

AC_ARG_WITH(qwt,
    [  --with-qwt=DIR     directory path to QWT installation ],
    [QWTHOME="$withval"
      AC_MSG_RESULT("select $withval as path to QWT")
    ])

AC_ARG_WITH(qwt_inc,
    [  --with-qwt_inc=DIR   directory path to QWT includes ],
    [QWT_INCDIR="$withval"
      AC_MSG_RESULT("select $withval as path to QWT includes")
    ])

libqwt_name=qwt
if test -z $QWTHOME; then
  AC_MSG_RESULT(QWTHOME not defined)
  AC_MSG_NOTICE(Trying native Qwt...)
  exist_ok=no	
  if test "x$exist_ok" = "xno"; then
     for d in /usr /usr/local ; do
        for extension in qwt-qt4 qwt; do
           AC_CHECK_FILE(${d}/lib${LIB_LOCATION_SUFFIX}/lib${extension}.so,exist_ok=yes,exist_ok=no)
           if test "x$exist_ok" = "xyes"; then
              QWTHOME=$d
              AC_MSG_RESULT(lib${extension}.so detected in $d/lib)
              libqwt_name=${extension}
              dnl  break, libqwt-qt4.so is choosen before libqwt.so since it is surely the Qt4 version.
              break
           fi
        done
        if test "x$exist_ok" = "xyes"; then
           break
        fi
     done
  fi
  if test "x$exist_ok" = "xno"; then
     for d in `echo $LD_LIBRARY_PATH | sed -e "s/:/ /g"` ; do
        if test -f $d/libqwt.so ; then
           AC_MSG_RESULT(libqwt.so detected in $d)
           QWTHOME=$d
           QWTHOME=`echo ${QWTHOME} | sed -e "s,[[^/]]*$,,;s,/$,,;s,^$,.,"`
           exist_ok=yes
           break
        fi
     done
  fi
  if test "x$exist_ok" = "xyes"; then
     if test -z $QWT_INCDIR; then
        QWT_INCDIR=$QWTHOME"/include/qwt-qt4"
        if test ! -f $QWT_INCDIR/qwt.h ; then
          QWT_INCDIR=/usr/include/qwt
        fi
        if test ! -f $QWT_INCDIR/qwt.h ; then
          QWT_INCDIR=$QWTHOME"/include"
        fi
        if test ! -f $QWT_INCDIR/qwt.h ; then
          QWT_INCDIR=/usr/lib/qt4/include/qwt
        fi
        if test ! -f $QWT_INCDIR/qwt.h ; then
          QWT_INCDIR=/usr/include/qwt-qt4
        fi
     fi
  else
     qwt_ok=no
  fi
else
  AC_MSG_NOTICE(Trying Qwt from $QWTHOME ...)
  if test -z $QWT_INCDIR; then
     QWT_INCDIR="$QWTHOME/include"
  fi   	
fi

if test "x$qwt_ok" = xno -o ! -d "$QWTHOME" ; then
  AC_MSG_RESULT(no)
  AC_MSG_WARN(qwt not found)
  qwt_ok=no
else
  CPPFLAGS_old=$CPPFLAGS
  CPPFLAGS="$CPPFLAGS $QT_INCLUDES -I$QWT_INCDIR"

  AC_CHECK_HEADER(qwt.h,qwt_ok=yes,qwt_ok=no) 
  CPPFLAGS=$CPPFLAGS_old

  AC_MSG_CHECKING(include of qwt headers)

  if test "x$qwt_ok" = xno ; then
    AC_MSG_RESULT(no)
    AC_MSG_WARN(qwt not found)
  else
    AC_MSG_RESULT(yes)
    QWT_INCLUDES=-I$QWT_INCDIR
  fi

  #
  # test Qwt libraries
  #
  if test "x$qwt_ok" = "xyes" ; then
    AC_MSG_CHECKING(linking qwt library)

    LIBS_old=$LIBS
    LIBS="$LIBS $QT_LIBS"
    if test "x$QWTHOME" = "x/usr" ; then
      LIBS="$LIBS -l${libqwt_name}"
    else
      LIBS="$LIBS -L$QWTHOME/lib -l${libqwt_name}"
    fi

    CXXFLAGS_old=$CXXFLAGS
    CXXFLAGS="$CXXFLAGS $QT_INCLUDES $QWT_INCLUDES"

    AC_CACHE_VAL(salome_cv_lib_qwt,[
      AC_TRY_LINK(
#include <QApplication>
#include <qwt_plot.h>
,     int n;
      char **s;
      QApplication a(n, s);
      QwtPlot p;
      p.resize( 600, 400 );
      p.show();
      a.exec();,
      eval "salome_cv_lib_qwt=yes",eval "salome_cv_lib_qwt=no")
    ])
    qwt_ok="$salome_cv_lib_qwt"

    if  test "x$qwt_ok" = "xno" ; then
      AC_MSG_RESULT(unable to link with qwt library)
      AC_MSG_RESULT(QWTHOME environment variable may be wrong)
    else
      AC_MSG_RESULT(yes)
      if test "x$QWTHOME" = "x/usr" ; then
        QWT_LIBS=" -l${libqwt_name}"
      else
        QWT_LIBS="-L$QWTHOME/lib -l${libqwt_name}"
      fi
    fi

    LIBS=$LIBS_old
    CXXFLAGS=$CXXFLAGS_old
  fi
fi

AC_SUBST(QWT_INCLUDES)
AC_SUBST(QWT_LIBS)

AC_LANG_RESTORE

AC_MSG_RESULT(for qwt: $qwt_ok)

# Save cache
AC_CACHE_SAVE

])dnl
dnl
