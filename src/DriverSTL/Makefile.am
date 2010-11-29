#  Copyright (C) 2007-2010  CEA/DEN, EDF R&D, OPEN CASCADE
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
#  See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

#  SMESH DriverSTL : driver to read and write 'stl' files
#  File   : Makefile.in
#  Author : Marc Tajchman (CEA)
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : SMESH
#  $Header$
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
	DriverSTL_R_SMDS_Mesh.h \
	DriverSTL_W_SMDS_Mesh.h \
	SMESH_DriverSTL.hxx

# Libraries targets
lib_LTLIBRARIES = libMeshDriverSTL.la

dist_libMeshDriverSTL_la_SOURCES = \
	DriverSTL_R_SMDS_Mesh.cxx \
	DriverSTL_W_SMDS_Mesh.cxx

# Executables targets
bin_PROGRAMS = STL_Test

dist_STL_Test_SOURCES = \
	STL_Test.cxx


# additionnal information to compil and link file
libMeshDriverSTL_la_CPPFLAGS = \
	$(KERNEL_CXXFLAGS) \
	$(CAS_CPPFLAGS) \
        $(VTK_INCLUDES) \
	$(BOOST_CPPFLAGS) \
	-I$(srcdir)/../Driver \
	-I$(srcdir)/../SMDS

libMeshDriverSTL_la_LDFLAGS  = \
	../Driver/libMeshDriver.la \
	../SMDS/libSMDS.la \
	$(CAS_LDPATH) -lTKernel -lTKSTL -lTKTopAlgo -lTKMesh

STL_Test_CPPFLAGS = \
	$(libMeshDriverSTL_la_CPPFLAGS)

STL_Test_LDADD = \
	libMeshDriverSTL.la \
	../Driver/libMeshDriver.la \
	../SMDS/libSMDS.la \
	$(KERNEL_LDFLAGS) -lOpUtil -lSALOMELocalTrace -lSALOMEBasics \
	$(CAS_LDPATH) -lTKernel -lTKSTL -lTKTopAlgo -lTKMesh -lTKBO

