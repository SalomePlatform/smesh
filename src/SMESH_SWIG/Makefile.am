#  Copyright (C) 2003  OPEN CASCADE, EADS/CCR, LIP6, CEA/DEN,
#  CEDRAT, EDF R&D, LEG, PRINCIPIA R&D, BUREAU VERITAS 
# 
#  This library is free software; you can redistribute it and/or 
#  modify it under the terms of the GNU Lesser General Public 
#  License as published by the Free Software Foundation; either 
#  version 2.1 of the License. 
# 
#  This library is distributed in the hope that it will be useful, 
#  but WITHOUT ANY WARRANTY; without even the implied warranty of 
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
#  Lesser General Public License for more details. 
# 
#  You should have received a copy of the GNU Lesser General Public 
#  License along with this library; if not, write to the Free Software 
#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA 
# 
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#
#
#
#  File   : Makefile.in
#  Author : Nicolas REJNERI, Paul RASCLE
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : SMESH
#  $Header$

include $(top_srcdir)/adm_local/unix/make_common_starter.am

# ===============================================================
# Swig targets
# ===============================================================
# (cf. http://www.geocities.com/foetsch/python/swig_linux.htm)
#
# Step 1: build the wrapping source files with swig
#
# libSALOME_LifeCycleCORBA.i -- swig --> swig_wrap.cpp
#                                        libSALOME_Swig.py
#
# Step 2: build the dynamic library from cpp built source files and
#         dependant libraries.
#
# swig_wrap.cpp -- gcc --> swig_wrap.o    |-- link --> _libSALOME_Swig.la
#                          +              |
#                          dependant libs |
#
# The file libSALOME_Swigcmodule.py will be installed in
# <prefix>/lib/python<version>/site-package/salome.
# The library will be installed in the same place.
#

# this option puts it to dist
#BUILT_SOURCES = swig_wrap.cpp

SWIG_FLAGS    = \
	@SWIG_FLAGS@ \
	-I$(srcdir) \
	-I$(srcdir)/../SMESHGUI

SWIG_SOURCES  = libSMESH_Swig.i

# Libraries targets

lib_LTLIBRARIES = libSMESH_Swigcmodule.la

nodist_pkgpython_DATA = libSMESH_Swig.py
libSMESH_Swig.py: swig_wrap.cpp

libSMESH_Swigcmodule_la_SOURCES = \
	$(BUILT_SOURCES) \
	$(SWIG_SOURCES) \
	../SMESHGUI/SMESHGUI_Swig.cxx

nodist_libSMESH_Swigcmodule_la_SOURCES = \
	swig_wrap.cpp

libSMESH_Swigcmodule_la_CPPFLAGS = \
	$(QT_INCLUDES) \
	$(PYTHON_INCLUDES) \
	$(CAS_CPPFLAGS) \
	$(VTK_INCLUDES) \
	$(OGL_INCLUDES) \
	$(KERNEL_CXXFLAGS) \
	$(GUI_CXXFLAGS) \
	$(MED_CXXFLAGS) \
	$(GEOM_CXXFLAGS) \
	$(CORBA_CXXFLAGS) \
	$(CORBA_INCLUDES) \
	$(BOOST_CPPFLAGS) \
	-I$(srcdir)/../SMESHGUI \
	-I$(top_builddir)/idl \
	-I$(top_builddir)/salome_adm/unix

libSMESH_Swigcmodule_la_LDFLAGS  = \
	../SMESHGUI/libSMESH.la \
	$(KERNEL_LDFLAGS) -lSalomeGenericObj -lSALOMELocalTrace \
	$(GUI_LDFLAGS) -lCAM -lsuit -lqtx -lSalomeApp -lstd -lEvent \
	$(PYTHON_LIBS) \
	$(QT_MT_LIBS)


swig_wrap.cpp : $(SWIG_SOURCES)
	$(SWIG) $(SWIG_FLAGS) -o $@ $<

CLEANFILES = \
	swig_wrap.cpp

# Scripts to be installed.
dist_salomescript_DATA= \
	libSMESH_Swig.py \
	smesh.py \
	batchmode_smesh.py \
	batchmode_mefisto.py \
	ex00_all.py \
	ex01_cube2build.py \
	ex02_cube2primitive.py \
	ex03_cube2partition.py \
	ex04_cube5tetraHexa.py \
	ex05_hole1build.py \
	ex06_hole1boolean.py \
	ex07_hole1partition.py \
	ex08_hole2build.py \
	ex09_grid4build.py \
	ex10_grid4geometry.py \
	ex11_grid3partition.py \
	ex12_grid17partition.py \
	ex13_hole1partial.py \
	ex14_cyl1holed.py \
	ex15_cyl2geometry.py \
	ex16_cyl2complementary.py \
	ex17_dome1.py \
	ex18_dome2.py \
	ex19_sphereINcube.py \
	ex21_lamp.py \
	SMESH_test.py\
	SMESH_test0.py\
	SMESH_test1.py \
	SMESH_test1_AndDisplay.py \
	SMESH_test2.py \
	SMESH_test3.py \
	SMESH_test4.py \
	SMESH_test5.py \
	SMESH_mechanic.py \
	SMESH_mechanic_tetra.py \
	SMESH_mechanic_editor.py \
	SMESH_mechanic_netgen.py \
	SMESH_fixation.py \
	SMESH_fixation_hexa.py \
	SMESH_fixation_tetra.py \
	SMESH_fixation_netgen.py \
	SMESH_box_tetra.py \
	SMESH_box2_tetra.py \
	SMESH_box3_tetra.py \
	SMESH_flight_skin.py \
	SMESH_Partition1_tetra.py\
	SMESH_controls.py \
	SMESH_freebord.py \
	SMESH_blocks.py \
	SMESH_BelongToGeom.py \
	SMESH_GroupFromGeom2.py \
	SMESH_box.py \
	SMESH_demo_hexa2_upd.py \
	SMESH_hexaedre.py \
	SMESH_Sphere.py \
	SMESH_GroupFromGeom.py \
	SMESH_Nut.py \
	SMESH_GroupLyingOnGeom.py \
	PAL_MESH_041_mesh.py \
	PAL_MESH_043_2D.py \
	PAL_MESH_043_3D.py \
	SMESH_reg.py

EXPORT_SHAREDPYSCRIPTS = \
	SMESH_shared_modules.py

install-exec-hook: $(libdir)/_libSMESH_Swig.so

$(libdir)/_libSMESH_Swig.so:
	ln -sf $(libdir)/libSMESH_Swigcmodule.so $@
