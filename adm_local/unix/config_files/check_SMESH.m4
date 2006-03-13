# Check availability of SMesh binary distribution
#
# Author : Nicolas REJNERI (OPEN CASCADE, 2003)
#

AC_DEFUN([CHECK_SMESH],[

AC_CHECKING(for SMesh)

SMesh_ok=no

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
      AC_PATH_PROG(TEMP, libSMESH_Swig.py)
      if test "x$TEMP" != "x" ; then
         SMESH_BIN_DIR=`dirname $TEMP`
         SMESH_DIR=`dirname $SMESH_BIN_DIR`
      fi
      
   fi
# 
fi

if test -f ${SMESH_DIR}/bin/salome/libSMESH_Swig.py ; then
   SMesh_ok=yes
   AC_MSG_RESULT(Using SMesh module distribution in ${SMESH_DIR})

   if test "x$SMESH_ROOT_DIR" == "x" ; then
      SMESH_ROOT_DIR=${SMESH_DIR}
   fi
   AC_SUBST(SMESH_ROOT_DIR)

else
   AC_MSG_WARN("Cannot find compiled SMesh module distribution")
fi

AC_MSG_RESULT(for SMesh: $SMesh_ok)
 
])dnl
 
