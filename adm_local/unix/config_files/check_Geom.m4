# Check availability of Geom binary distribution
#
# Author : Nicolas REJNERI (OPEN CASCADE, 2003)
#

AC_DEFUN([CHECK_GEOM],[

AC_CHECKING(for Geom)

Geom_ok=no

AC_ARG_WITH(geom,
	    [  --with-geom=DIR root directory path of GEOM installation ],
	    GEOM_DIR="$withval",GEOM_DIR="")

if test "x$GEOM_DIR" == "x" ; then

# no --with-geom-dir option used

   if test "x$GEOM_ROOT_DIR" != "x" ; then

    # GEOM_ROOT_DIR environment variable defined
      GEOM_DIR=$GEOM_ROOT_DIR

   fi
# 
fi

if test -f ${GEOM_DIR}/lib/salome/libGEOMClient.so ; then
   Geom_ok=yes
   AC_MSG_RESULT(Using Geom module distribution in ${GEOM_DIR})

   if test "x$GEOM_ROOT_DIR" == "x" ; then
      GEOM_ROOT_DIR=${GEOM_DIR}
   fi
   AC_SUBST(GEOM_ROOT_DIR)

else
   AC_MSG_WARN("Cannot find compiled Geom module distribution")
fi

AC_MSG_RESULT(for Geom: $Geom_ok)
 
])dnl
 
