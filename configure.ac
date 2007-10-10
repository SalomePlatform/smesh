#
# Author : Marc Tajchman (CEA)
# Date : 28/06/2001
# Modified by : Patrick GOLDBRONN (CEA)
# Modified by : Marc Tajchman (CEA)
# Modified by : Alexander BORODIN (OCN) - autotools usage
#
# Created from configure.in.base
#


AC_INIT([Salome2 Project SMESH module], [4.0.0], [webmaster.salome@opencascade.com], [salome])
AC_CONFIG_AUX_DIR(salome_adm/unix/config_files)
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE

XVERSION=`echo $VERSION | awk -F. '{printf("0x%02x%02x%02x",$1,$2,$3)}'`
AC_SUBST(XVERSION)

# set up MODULE_NAME variable for dynamic construction of directories (resources, etc.)
MODULE_NAME=smesh
AC_SUBST(MODULE_NAME)

echo
echo ---------------------------------------------
echo Initialize source and build root directories
echo ---------------------------------------------
echo

dnl
dnl Initialize source and build root directories
dnl

ROOT_BUILDDIR=`pwd`
ROOT_SRCDIR=`echo $0 | sed -e "s,[[^/]]*$,,;s,/$,,;s,^$,.,"`
cd $ROOT_SRCDIR
ROOT_SRCDIR=`pwd`
cd $ROOT_BUILDDIR

AC_SUBST(ROOT_SRCDIR)
AC_SUBST(ROOT_BUILDDIR)

echo
echo Source root directory : $ROOT_SRCDIR
echo Build  root directory : $ROOT_BUILDDIR
echo
echo

if test -z "$AR"; then
   AC_CHECK_PROGS(AR,ar xar,:,$PATH)
fi
AC_SUBST(AR)

dnl Export the AR macro so that it will be placed in the libtool file
dnl correctly.
export AR

echo
echo ---------------------------------------------
echo testing make
echo ---------------------------------------------
echo

AC_PROG_MAKE_SET
AC_PROG_INSTALL
dnl
dnl libtool macro check for CC, LD, NM, LN_S, RANLIB, STRIP + pour les librairies dynamiques !

echo
echo ---------------------------------------------
echo Coniguring production
echo ---------------------------------------------
echo
AC_ENABLE_DEBUG(yes)
AC_DISABLE_PRODUCTION

echo ---------------------------------------------
echo testing libtool
echo ---------------------------------------------

dnl first, we set static to no!
dnl if we want it, use --enable-static
AC_ENABLE_STATIC(no)

AC_LIBTOOL_DLOPEN
AC_PROG_LIBTOOL

dnl Fix up the INSTALL macro if it s a relative path. We want the
dnl full-path to the binary instead.
case "$INSTALL" in
   *install-sh*)
      INSTALL='\${ROOT_BUILDDIR}'/salome_adm/unix/config_files/install-sh
      ;;
esac

echo
echo ---------------------------------------------
echo testing C/C++
echo ---------------------------------------------
echo

cc_ok=no
dnl inutil car libtool
dnl AC_PROG_CC
AC_PROG_CXX
AC_CXX_WARNINGS
AC_CXX_TEMPLATE_OPTIONS
AC_DEPEND_FLAG
# AC_CC_WARNINGS([ansi])
cc_ok=yes

echo
echo ---------------------------------------------
echo testing Fortran
echo ---------------------------------------------
echo

fortran_ok=no
AC_PROG_F77
AC_F77_LIBRARY_LDFLAGS
AC_PROG_FC
AC_FC_LIBRARY_LDFLAGS
if test "X$FC" != "X" ; then
   fortran_ok=yes
fi

dnl AM_CONDITIONAL( USE_GFORTRAN, [test "$F77" = "gfortran"])

echo

dnl Library libdl :
AC_CHECK_LIB(dl,dlopen)

dnl add library libm :
AC_CHECK_LIB(m,ceil)

dnl
dnl Well we use sstream which is not in gcc pre-2.95.3
dnl We must test if it exists. If not, add it in include !
dnl

AC_CXX_HAVE_SSTREAM

dnl
dnl ---------------------------------------------
dnl testing MPICH
dnl ---------------------------------------------
dnl

CHECK_MPICH

echo
echo ---------------------------------------------
echo testing LEX \& YACC
echo ---------------------------------------------
echo

lex_yacc_ok=no
AC_PROG_YACC
AC_PROG_LEX
lex_yacc_ok=yes

echo
echo ---------------------------------------------
echo testing python
echo ---------------------------------------------
echo

CHECK_PYTHON

dnl echo
dnl echo ---------------------------------------------
dnl echo testing java
dnl echo ---------------------------------------------
dnl echo

dnl CHECK_JAVA

echo
echo ---------------------------------------------
echo testing swig
echo ---------------------------------------------
echo

AM_PATH_PYTHON(2.3)
CHECK_SWIG

echo
echo ---------------------------------------------
echo testing threads
echo ---------------------------------------------
echo

ENABLE_PTHREADS

echo
echo ---------------------------------------------
echo testing msg2qm
echo ---------------------------------------------
echo

CHECK_MSG2QM

if test "x${GUI_DISABLE_CORBA}" != "xyes" ; then
    echo
    echo ---------------------------------------------
    echo testing omniORB
    echo ---------------------------------------------
    echo

    CHECK_OMNIORB

dnl echo
dnl echo ---------------------------------------------
dnl echo testing mico
dnl echo ---------------------------------------------
dnl echo

dnl CHECK_MICO

    echo
    echo ---------------------------------------------
    echo default ORB : omniORB
    echo ---------------------------------------------
    echo

    DEFAULT_ORB=omniORB

    echo
    echo ---------------------------------------------
    echo testing Corba
    echo ---------------------------------------------
    echo

    CHECK_CORBA

    AC_SUBST_FILE(CORBA)
    corba=make_$ORB
    CORBA=adm_local/unix/$corba

fi


SMESH_WITH_GUI=yes

AM_CONDITIONAL(SMESH_ENABLE_GUI, [test "${SMESH_WITH_GUI}" = "yes"])

if test "${SMESH_WITH_GUI}" = "yes"; then
    echo
    echo ---------------------------------------------
    echo testing openGL
    echo ---------------------------------------------
    echo

    CHECK_OPENGL

    echo
    echo ---------------------------------------------
    echo testing QT
    echo ---------------------------------------------
    echo

    CHECK_QT

    echo
    echo ---------------------------------------------
    echo testing VTK
    echo ---------------------------------------------
    echo

    CHECK_VTK

    echo
    echo ---------------------------------------------
    echo Testing GUI
    echo ---------------------------------------------
    echo

    CHECK_SALOME_GUI

    echo
    echo ---------------------------------------------
    echo Testing full GUI
    echo ---------------------------------------------
    echo

    CHECK_CORBA_IN_GUI
    if test "x${CORBA_IN_GUI}" != "xyes"; then
      echo "failed : For configure SMESH module necessary full GUI !"
      exit
    fi
fi

echo
echo ---------------------------------------------
echo testing HDF5
echo ---------------------------------------------
echo

CHECK_HDF5

echo
echo ---------------------------------------------
echo BOOST Library
echo ---------------------------------------------
echo

CHECK_BOOST

echo
echo ---------------------------------------------
echo Testing OpenCascade
echo ---------------------------------------------
echo

CHECK_CAS

echo
echo ---------------------------------------------
echo Testing qwt
echo ---------------------------------------------
echo

CHECK_QWT

echo
echo ---------------------------------------------
echo Testing html generators
echo ---------------------------------------------
echo

CHECK_HTML_GENERATORS

echo
echo ---------------------------------------------
echo Testing Kernel
echo ---------------------------------------------
echo

CHECK_KERNEL

echo
echo ---------------------------------------------
echo Testing Geom
echo ---------------------------------------------
echo

CHECK_GEOM

echo
echo ---------------------------------------------
echo Testing Med
echo ---------------------------------------------
echo

CHECK_MED

CHECK_PLATFORM

echo
echo ---------------------------------------------
echo Summary
echo ---------------------------------------------
echo

echo Configure
variables="cc_ok fortran_ok boost_ok lex_yacc_ok python_ok swig_ok threads_ok OpenGL_ok qt_ok vtk_ok hdf5_ok omniORB_ok occ_ok doxygen_ok graphviz_ok qwt_ok Kernel_ok Geom_ok Med_ok"

for var in $variables
do
   printf "   %10s : " `echo \$var | sed -e "s,_ok,,"`
   eval echo \$$var
done

echo
echo "Default ORB   : $DEFAULT_ORB"
echo

dnl We don t need to say when we re entering directories if we re using
dnl GNU make becuase make does it for us.
if test "X$GMAKE" = "Xyes"; then
   AC_SUBST(SETX) SETX=":"
else
   AC_SUBST(SETX) SETX="set -x"
fi

dnl AM_CONDITIONAL(GUI_ENABLE_CORBA, [test "$GUI_DISABLE_CORBA" = no])
dnl AM_CONDITIONAL(ENABLE_PYCONSOLE, [test "$DISABLE_PYCONSOLE" = no])
dnl AM_CONDITIONAL(ENABLE_GLVIEWER, [test "$DISABLE_GLVIEWER" = no])
dnl AM_CONDITIONAL(ENABLE_PLOT2DVIEWER, [test "$DISABLE_PLOT2DVIEWER" = no])
dnl AM_CONDITIONAL(ENABLE_SUPERVGRAPHVIEWER, [test "$DISABLE_SUPERVGRAPHVIEWER" = no])
dnl AM_CONDITIONAL(ENABLE_OCCVIEWER, [test "$DISABLE_OCCVIEWER" = no])
dnl AM_CONDITIONAL(ENABLE_VTKVIEWER, [test "$DISABLE_VTKVIEWER" = no])
dnl AM_CONDITIONAL(ENABLE_SALOMEOBJECT, [test "$DISABLE_SALOMEOBJECT" = no])

echo
echo ---------------------------------------------
echo copying resource files, shell scripts, and
echo xml files
echo ---------------------------------------------
echo


dnl copy shells and utilities contained in the bin directory
dnl excluding .in files (treated in AC-OUTPUT below) and CVS
dnl directory

mkdir -p bin/salome
cd bin/salome

for i in $ROOT_SRCDIR/bin/*
do
  local_bin=`echo $i | sed -e "s,$ROOT_SRCDIR,.,"`
  case "$local_bin" in
        *.in | *~)                    ;;
        ./bin/CVS | ./bin/salome)                    ;;
        *) $INSTALL $i . ; echo $local_bin ;;
  esac
done
cd $ROOT_BUILDDIR

echo
echo ---------------------------------------------
echo generating Makefiles and configure files
echo ---------------------------------------------
echo

AC_OUTPUT_COMMANDS([ \
  chmod +x ./bin/*; \
  chmod +x ./bin/salome/*; \
])

# This list is initiated using autoscan and must be updated manually
# when adding a new file <filename>.in to manage. When you execute
# autoscan, the Makefile list is generated in the output file configure.scan.
# This could be helpfull to update de configuration.
AC_OUTPUT([ \
  ./salome_adm/unix/SALOMEconfig.h \
  ./adm_local/Makefile \
  ./adm_local/unix/Makefile \
  ./adm_local/unix/config_files/Makefile \
  ./bin/VERSION \
  ./bin/Makefile \
  ./SMESH_version.h \
  ./doc/Makefile \
  ./doc/salome/Makefile \
  ./doc/salome/gui/Makefile \
  ./doc/salome/gui/SMESH/doxyfile \
  ./doc/salome/gui/SMESH/doxyfile_py \
  ./doc/salome/tui/Makefile \
  ./doc/salome/tui/SMESH/doxyfile \
  ./doc/salome/tui/SMESH/sources/static/tree.js \
  ./src/Makefile \
  ./src/Controls/Makefile \
  ./src/Driver/Makefile \
  ./src/DriverDAT/Makefile \
  ./src/DriverMED/Makefile \
  ./src/DriverSTL/Makefile \
  ./src/DriverUNV/Makefile \
  ./src/MEFISTO2/Makefile \
  ./src/OBJECT/Makefile \
  ./src/SMDS/Makefile \
  ./src/SMESH/Makefile \
  ./src/SMESHClient/Makefile \
  ./src/SMESHDS/Makefile \
  ./src/SMESHFiltersSelection/Makefile \
  ./src/SMESHGUI/Makefile \
  ./src/SMESH_I/Makefile \
  ./src/SMESH_SWIG/Makefile \
  ./src/SMESH_SWIG_WITHIHM/Makefile \
  ./src/StdMeshers/Makefile \
  ./src/StdMeshersGUI/Makefile \
  ./src/StdMeshers_I/Makefile \
  ./resources/Makefile \
  ./resources/SMESHCatalog.xml \
  ./idl/Makefile \
  Makefile
])
