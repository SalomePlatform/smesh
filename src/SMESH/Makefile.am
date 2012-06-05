# Copyright (C) 2007-2012  CEA/DEN, EDF R&D, OPEN CASCADE
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
#
# See http://www.salome-platform.org/ or email : webmaster.salome@opencascade.com
#

#  SMESH SMESH : implementaion of SMESH idl descriptions
#  File   : Makefile.in
#  Author : Paul RASCLE, EDF
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : SMESH

include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
	SMESH_Gen.hxx \
	SMESH_Mesh.hxx \
	SMESH_subMesh.hxx \
	SMESH_subMeshEventListener.hxx \
	SMESH_Hypothesis.hxx \
	SMESH_HypoFilter.hxx \
	SMESH_Algo.hxx \
	SMESH_0D_Algo.hxx \
	SMESH_1D_Algo.hxx \
	SMESH_2D_Algo.hxx \
	SMESH_3D_Algo.hxx \
	SMESH_Group.hxx \
	SMESH_MeshEditor.hxx \
	SMESH_Pattern.hxx \
	SMESH_MesherHelper.hxx \
	SMESH_ProxyMesh.hxx \
	SMESH_SMESH.hxx

# Libraries targets

lib_LTLIBRARIES = libSMESHimpl.la

dist_libSMESHimpl_la_SOURCES = \
	memoire.h \
	SMESH_Gen.cxx \
	SMESH_Mesh.cxx \
	SMESH_subMesh.cxx \
	SMESH_Hypothesis.cxx \
	SMESH_Algo.cxx \
	SMESH_0D_Algo.cxx \
	SMESH_1D_Algo.cxx \
	SMESH_2D_Algo.cxx \
	SMESH_3D_Algo.cxx \
	SMESH_Group.cxx \
	SMESH_MeshEditor.cxx \
	SMESH_Pattern.cxx \
	SMESH_HypoFilter.cxx \
	SMESH_ProxyMesh.cxx \
	SMESH_MesherHelper.cxx

# additionnal information to compile and link file
libSMESHimpl_la_CPPFLAGS = \
	$(KERNEL_CXXFLAGS) \
	$(CAS_CPPFLAGS) \
	$(MED_CXXFLAGS) \
	$(GEOM_CXX_FLAGS) \
        $(VTK_INCLUDES) \
	$(BOOST_CPPFLAGS) \
	@HDF5_INCLUDES@ \
	-I$(srcdir)/../Controls \
	-I$(srcdir)/../Driver \
	-I$(srcdir)/../DriverDAT \
	-I$(srcdir)/../DriverMED \
	-I$(srcdir)/../DriverUNV \
	-I$(srcdir)/../DriverSTL \
	-I$(srcdir)/../DriverCGNS \
	-I$(srcdir)/../SMDS \
	-I$(srcdir)/../SMESHDS \
	-I$(srcdir)/../SMESHUtils

if WITH_CGNS
  DriverCGNS_LIB = ../DriverCGNS/libMeshDriverCGNS.la
endif

libSMESHimpl_la_LDFLAGS = \
	../SMESHDS/libSMESHDS.la \
	../Controls/libSMESHControls.la \
	../DriverDAT/libMeshDriverDAT.la \
	../DriverSTL/libMeshDriverSTL.la \
	../DriverMED/libMeshDriverMED.la \
	../DriverUNV/libMeshDriverUNV.la \
	$(DriverCGNS_LIB) \
	../SMESHUtils/libSMESHUtils.la \
	$(BOOST_LIB_THREAD)  \
	$(GEOM_LDFLAGS) -lNMTTools \
	$(CAS_LDPATH) -lTKShHealing -lTKPrim -lTKG2d
