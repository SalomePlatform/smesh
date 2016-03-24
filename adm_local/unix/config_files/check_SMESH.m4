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

# Check availability of SMesh binary distribution
#
# Author : Nicolas REJNERI (OPEN CASCADE, 2003)
#

AC_DEFUN([CHECK_SMESH],[

AC_CHECKING(for SMesh)

SMesh_ok=no

SMESH_LDFLAGS=""
SMESH_CXXFLAGS=""

AC_ARG_WITH(smesh,
	    [  --with-smesh=DIR root directory path of SMESH installation ],
	    SMESH_DIR="$withval",SMESH_DIR="")

if test "x$SMESH_DIR" == "x" ; then

# no --with-smesh option used

   if test "x$SMESH_ROOT_DIR" != "x" ; then

    # SMESH_ROOT_DIR environment variable defined
      SMESH_DIR=$SMESH_ROOT_DIR

   else

    # search SMESH binaries in PATH variable
      #CCRTAC_PATH_PROG(TEMP, libSMESH_Swig.py)
      #AC_PATH_PROG(TEMP, MED_Test)
      AC_PATH_PROG(TEMP, smesh.py)
      if test "x$TEMP" != "x" ; then
         SMESH_BIN_DIR=`dirname $TEMP`
         SMESH_DIR=`dirname $SMESH_BIN_DIR`
      fi
      
   fi
# 
fi

#CCRTif test -f ${SMESH_DIR}/bin/salome/libSMESH_Swig.py ; then
#if test -f ${SMESH_DIR}/bin/salome/MED_Test ; then
if test -f ${SMESH_DIR}/bin/salome/SMESH_test.py ; then
   SMesh_ok=yes
   AC_MSG_RESULT(Using SMesh module distribution in ${SMESH_DIR})

   if test "x$SMESH_ROOT_DIR" == "x" ; then
      SMESH_ROOT_DIR=${SMESH_DIR}
   fi
   AC_SUBST(SMESH_ROOT_DIR)

   SMESH_LDFLAGS=-L${SMESH_DIR}/lib${LIB_LOCATION_SUFFIX}/salome
   SMESH_CXXFLAGS=-I${SMESH_DIR}/include/salome

   AC_SUBST(SMESH_LDFLAGS)
   AC_SUBST(SMESH_CXXFLAGS)   

else
   AC_MSG_WARN("Cannot find compiled SMesh module distribution")
fi

AC_MSG_RESULT(for SMesh: $SMesh_ok)
 
])dnl
 
