# Check availability of Med binary distribution
#
# Author : Nicolas REJNERI (OPEN CASCADE, 2003)
#

AC_DEFUN([CHECK_MED],[

AC_CHECKING(for Med)

Med_ok=no

AC_ARG_WITH(med,
	    [  --with-med=DIR root directory path of MED installation ],
	    MED_DIR="$withval",MED_DIR="")

if test "x$MED_DIR" == "x" ; then

# no --with-med-dir option used

   if test "x$MED_ROOT_DIR" != "x" ; then

    # MED_ROOT_DIR environment variable defined
      MED_DIR=$MED_ROOT_DIR

   fi
# 
fi

if test -f ${MED_DIR}/idl/salome/MED.idl ; then
   Med_ok=yes
   AC_MSG_RESULT(Using Med module distribution in ${MED_DIR})

   if test "x$MED_ROOT_DIR" == "x" ; then
      MED_ROOT_DIR=${MED_DIR}
   fi
   AC_SUBST(MED_ROOT_DIR)

else
   AC_MSG_WARN("Cannot find Med module sources")
fi
  
AC_MSG_RESULT(for Med: $Med_ok)
 
])dnl
 
