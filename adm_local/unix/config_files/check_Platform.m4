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

AC_DEFUN([CHECK_PLATFORM],[
AC_REQUIRE([AC_PROG_CC])dnl
AC_REQUIRE([AC_PROG_CPP])dnl

AC_CHECKING(for Platform)

AC_SUBST(PLATFORM_INCLUDES)

PLATFORM_INCLUDES="PCLINUX"
SUFFIXES=""

f77int="F77INT32"
case  $host_os in
   irix5.* | irix6.* | osf4.* | osf5.* | linux*  )

        linux64="true"
        expr "$host_os" : 'linux' >/dev/null && test ! x"$host_cpu" = x"x86_64" && linux64="false"
	if test ! x"$linux64" = "xfalse" ; then
	  echo "$as_me:$LINENO: checking for 64bits integers size in F77/F90" >&5
echo $ECHO_N "checking for 64bits integers size in F77/F90... $ECHO_C" >&6
	  # Check whether --enable-int64 or --disable-int64 was given.
if test "${enable_int64+set}" = set; then
  enableval="$enable_int64"

fi;
	  case "X-$enable_int64" in
	    X-no)
	     echo "$as_me:$LINENO: result: \"disabled\"" >&5
echo "${ECHO_T}\"disabled\"" >&6
	     SUFFIXES="_32"
	     ;;
	    *)
	     echo "$as_me:$LINENO: result: \"enabled\"" >&5
echo "${ECHO_T}\"enabled\"" >&6
	     SUFFIXES=""
	     f77int="F77INT64"
	     ;;
	  esac
	fi
     ;;
   *)
     ;;
esac

case $host_os in
    linux*)
        test x"$linux64" = x"true" && \
          MACHINE="PCLINUX64${SUFFIXES}" || \
	MACHINE=PCLINUX
	;;
    hpux*)
	MACHINE=HP9000
	;;
    aix4.*)
	MACHINE=RS6000
	host_os_novers=aix4.x
	;;
    irix5.*)
	MACHINE="IRIX64${SUFFIXES}"
	host_os_novers=irix5.x
	;;
    irix6.*)
	MACHINE="IRIX64${SUFFIXES}"
	host_os_novers=irix6.x
	;;
    osf4.*)
	MACHINE="OSF1${SUFFIXES}"
	host_os_novers=osf4.x
	;;
    osf5.*)
	MACHINE="OSF1${SUFFIXES}"
	 host_os_novers=osf5.x
	 ;;
    solaris2.*)
	MACHINE=SUN4SOL2
	 host_os_novers=solaris2.x
	 ;;
    uxpv*)
	MACHINE=VPP5000
	 ;;
    *)
	MACHINE=
	 host_os_novers=$host_os
	 ;;
esac

case $host_cpu in
    ia64*)
	MACHINE="PCLINUX64"
	;;
esac

PLATFORM_INCLUDES=" -D$MACHINE "

])dnl
