AC_DEFUN([CHECK_NETGEN],[

AC_REQUIRE([AC_PROG_CXX])dnl
AC_REQUIRE([AC_PROG_CXXCPP])dnl

AC_CHECKING(for Netgen Libraries)

AC_LANG_SAVE
AC_LANG_CPLUSPLUS

AC_ARG_WITH(netgen,
	    --with-netgen=DIR root directory path of NETGEN installation,
	    WITHNETGEN="yes",WITHNETGEN="no")

NETGEN_INCLUDES=""
NETGEN_LIBS=""

Netgen_ok=no

if test "$WITHNETGEN" = yes; then

  echo
  echo
  echo -------------------------------------------------
  echo You are about to choose to use somehow the
  echo Netgen Library to generate Tetrahedric mesh.
  echo
  echo WARNING
  echo ----------------------------------------------------------
  echo ----------------------------------------------------------
  echo You are strongly advised to consult the file
  echo SMESH_SRC/src/NETGEN/ReadMeForNgUsers, particularly about
  echo assumptions made on the installation of the Netgen
  echo application and libraries.
  echo Ask your system administrator for those details.
  echo ----------------------------------------------------------
  echo ----------------------------------------------------------
  echo 
  echo

  NETGEN_HOME=$withval

  if test "$NETGEN_HOME"; then
    NETGEN_INCLUDES="-I$NETGEN_HOME/include"
    NETGEN_LIBS_DIR="$NETGEN_HOME/lib/LINUX"
    NETGEN_LIBS="-L$NETGEN_LIBS_DIR"
  fi

  CPPFLAGS_old="$CPPFLAGS"
  CPPFLAGS="$NETGEN_INCLUDES $CPPFLAGS"
  CXXFLAGS_old="$CXXFLAGS"
  CXXFLAGS="$NETGEN_INCLUDES $CXXFLAGS"

  AC_MSG_CHECKING(for Netgen header file)

  AC_CHECK_HEADER(nglib.h,Netgen_ok=yes,Netgen_ok=no)
  CPPFLAGS="$CPPFLAGS_old"
  CXXFLAGS="$CXXFLAGS_old"

  if test "$WITHNETGEN" = "yes";then
    NETGEN_LIBS="-L. -lNETGEN"

    AC_MSG_CHECKING(for Netgen libraries)

    CPPFLAGS_old="$CPPFLAGS"
    CPPFLAGS="$NETGEN_INCLUDES $CPPFLAGS"
    CXXFLAGS_old="$CXXFLAGS"
    CXXFLAGS="$NETGEN_INCLUDES $CXXFLAGS"

    LDFLAGS_old="$LDFLAGS"
    LDFLAGS="$NETGEN_LIBS $LDFLAGS"

    AC_TRY_COMPILE(#include <iostream.h>
#include <fstream.h>
#include "nglib.h"
,Ng_Init();
 Ng_Exit();,Netgen_ok=yes;ar x "$NETGEN_LIBS_DIR"/libnginterface.a;
            ar x "$NETGEN_LIBS_DIR"/libcsg.a;
            ar x "$NETGEN_LIBS_DIR"/libgprim.a;
            ar x "$NETGEN_LIBS_DIR"/libmesh.a;
            ar x "$NETGEN_LIBS_DIR"/libopti.a;
            ar x "$NETGEN_LIBS_DIR"/libgen.a;
            ar x "$NETGEN_LIBS_DIR"/libla.a;
            ar x "$NETGEN_LIBS_DIR"/libstlgeom.a;
            ar x "$NETGEN_LIBS_DIR"/libgeom2d.a;
            "$CXX" -shared linopt.o bfgs.o linsearch.o global.o bisect.o meshtool.o refine.o ruler3.o improve3.o adfront3.o tetrarls.o prism2rls.o pyramidrls.o pyramid2rls.o netrule3.o ruler2.o meshclass.o improve2.o adfront2.o netrule2.o triarls.o geomsearch.o secondorder.o meshtype.o parser3.o quadrls.o specials.o parser2.o meshing2.o meshing3.o meshfunc.o localh.o improve2gen.o delaunay.o boundarylayer.o msghandler.o meshfunc2d.o smoothing2.o smoothing3.o topology.o curvedelems.o clusters.o zrefine.o ngexception.o geomtest3d.o geom2d.o geom3d.o adtree.o transform3d.o geomfuncs.o polynomial.o densemat.o vector.o basemat.o sparsmat.o algprim.o brick.o manifold.o bspline2d.o meshsurf.o csgeom.o polyhedra.o curve2d.o singularref.o edgeflw.o solid.o explicitcurve2d.o specpoin.o gencyl.o revolution.o genmesh.o spline3d.o surface.o identify.o triapprox.o meshstlsurface.o stlline.o stltopology.o stltool.o stlgeom.o stlgeomchart.o stlgeommesh.o table.o optmem.o spbita2d.o hashtabl.o sort.o flags.o seti.o bitarray.o array.o symbolta.o mystring.o moveablemem.o spline2d.o splinegeometry2.o ngnewdelete.o nglib.o -o libNETGEN.so;
            rm -rf adfront2.o adfront3.o adtree.o algprim.o array.o basemat.o bfgs.o bisect.o bitarray.o boundarylayer.o brick.o bspline2d.o clusters.o csgeom.o csgparser.o curve2d.o curvedelems.o delaunay.o densemat.o dynamicmem.o edgeflw.o explicitcurve2d.o extrusion.o flags.o gencyl.o genmesh.o geom2dmesh.o geom2d.o geom3d.o geomfuncs.o geomsearch.o geomtest3d.o global.o hashtabl.o hprefinement.o identify.o importsolution.o improve2gen.o improve2.o improve3.o linopt.o linsearch.o localh.o manifold.o meshclass.o meshfunc2d.o meshfunc.o meshing2.o meshing3.o meshstlsurface.o meshsurf.o meshtool.o meshtype.o moveablemem.o msghandler.o mystring.o netrule2.o netrule3.o ngexception.o nglib.o ngnewdelete.o optmem.o parser2.o parser3.o parthreads.o polyhedra.o polynomial.o prism2rls.o pyramid2rls.o pyramidrls.o quadrls.o readuser.o refine.o revolution.o ruler2.o ruler3.o secondorder.o seti.o singularref.o smoothing2.o smoothing3.o solid.o sort.o sparsmat.o spbita2d.o specials.o specpoin.o spline2d.o spline3d.o splinegeometry2.o stlgeomchart.o stlgeommesh.o stlgeom.o stlline.o stltool.o stltopology.o surface.o symbolta.o table.o tetrarls.o topology.o transform3d.o triapprox.o triarls.o vector.o writeabaqus.o writediffpack.o writefeap.o writefluent.o writepermas.o writetecplot.o writetochnog.o writeuser.o wuchemnitz.o zrefine.o,
            Netgen_ok=no)

    AC_CACHE_VAL(salome_netgen_lib,[
                 AC_TRY_LINK(
#include <iostream.h>
#include <fstream.h>
#include "nglib.h"
,Ng_Init();
 Ng_Exit();,
    eval "salome_netgen_lib=yes";rm -rf libNETGEN.so,eval "salome_netgen_lib=no";rm -rf libNETGEN.so)
  ])
  Netgen_ok="$salome_netgen_lib"


    LDFLAGS="$LDFLAGS_old"
    CPPFLAGS="$CPPFLAGS_old"
    CXXFLAGS="$CXXFLAGS_old"
  fi


if test "x$Netgen_ok" = xno ; then
  AC_MSG_RESULT(no)
  AC_MSG_WARN(Netgen libraries not found or not properly installed)
else
  AC_MSG_RESULT(yes)
  NETGEN_LIBS="-lNETGEN"
fi
fi
AC_SUBST(NETGEN_INCLUDES)
AC_SUBST(NETGEN_LIBS)
AC_SUBST(NETGEN_LIBS_DIR)
AC_SUBST(WITHNETGEN)

AC_LANG_RESTORE

])dnl
