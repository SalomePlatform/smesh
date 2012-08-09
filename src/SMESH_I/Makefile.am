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

#  SMESH SMESH_I : idl implementation based on 'SMESH' unit's calsses
#  File   : Makefile.in
#  Author : Paul RASCLE, EDF
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : SMESH
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
	SMESH_Gen_i.hxx \
	SMESH_Algo_i.hxx \
	SMESH_0D_Algo_i.hxx \
	SMESH_1D_Algo_i.hxx \
	SMESH_2D_Algo_i.hxx \
	SMESH_3D_Algo_i.hxx \
	SMESH_subMesh_i.hxx \
	SMESH_Mesh_i.hxx \
	SMESH_Hypothesis_i.hxx \
	SMESH_PythonDump.hxx \
	SMESH_Group_i.hxx \
	SMESH_MEDMesh_i.hxx \
	SMESH_Filter_i.hxx \
	SMESH_MeshEditor_i.hxx \
	SMESH_MEDFamily_i.hxx \
	SMESH_MEDSupport_i.hxx \
	SMESH_Pattern_i.hxx \
	SMESH_2smeshpy.hxx \
	SMESH_NoteBook.hxx \
	SMESH_Measurements_i.hxx \
	SMESH_PreMeshInfo.hxx \
	SMESH.hxx

# Scripts to be installed.
dist_salomescript_DATA= \
	smeshpy.py

# Libraries targets

lib_LTLIBRARIES = libSMESHEngine.la

dist_libSMESHEngine_la_SOURCES = \
	SMESH_Gen_i.cxx \
	SMESH_Gen_i_1.cxx \
	SMESH_DumpPython.cxx \
	SMESH_Mesh_i.cxx \
	SMESH_MEDMesh_i.cxx \
	SMESH_MEDFamily_i.cxx \
	SMESH_MEDSupport_i.cxx \
	SMESH_subMesh_i.cxx \
	SMESH_MeshEditor_i.cxx \
	SMESH_Hypothesis_i.cxx \
	SMESH_Algo_i.cxx \
	SMESH_0D_Algo_i.cxx \
	SMESH_1D_Algo_i.cxx \
	SMESH_2D_Algo_i.cxx \
	SMESH_3D_Algo_i.cxx \
	SMESH_Filter_i.cxx \
	SMESH_Group_i.cxx \
	SMESH_Pattern_i.cxx \
	SMESH_2smeshpy.cxx \
	SMESH_NoteBook.cxx \
	SMESH_Measurements_i.cxx \
	SMESH_PreMeshInfo.cxx

# Executables targets
bin_PROGRAMS = SMESHEngine

dist_SMESHEngine_SOURCES = \
	SMESHEngine.cxx 

# additionnal information to compil and link file
libSMESHEngine_la_CPPFLAGS = \
	$(CORBA_CXXFLAGS) \
	$(CORBA_INCLUDES) \
	$(CAS_CPPFLAGS) \
	@HDF5_INCLUDES@ \
        $(VTK_INCLUDES) \
	$(BOOST_CPPFLAGS) \
	$(KERNEL_CXXFLAGS) \
	$(GUI_CXXFLAGS) \
	$(MED_CXXFLAGS) \
	$(GEOM_CXXFLAGS) \
	-I$(srcdir)/../Controls \
	-I$(srcdir)/../SMDS \
	-I$(srcdir)/../SMESHDS \
	-I$(srcdir)/../Driver \
	-I$(srcdir)/../DriverMED \
	-I$(srcdir)/../DriverCGNS \
	-I$(srcdir)/../SMESH \
	-I$(srcdir)/../SMESHUtils \
	-I$(top_builddir)/idl

libSMESHEngine_la_LDFLAGS  = \
	../../idl/libSalomeIDLSMESH.la \
	../SMESH/libSMESHimpl.la \
	../SMDS/libSMDS.la \
	../SMESHDS/libSMESHDS.la \
	../Controls/libSMESHControls.la \
	$(KERNEL_LDFLAGS) \
	-lSalomeContainer \
	-lSalomeNS \
	-lRegistry \
	-lSalomeHDFPersist \
	-lSalomeLifeCycleCORBA \
	-lTOOLSDS \
	-lSalomeGenericObj \
	-lSalomeIDLKernel \
	-lSALOMELocalTrace \
	$(MED_LDFLAGS) \
	-lMEDWrapper \
	-lMEDWrapper_V2_2 \
        -lSalomeIDLMED \
	$(CAS_LDPATH) \
	-lTKCDF \
	-lTKBO \
	-lTKShHealing \
	$(GEOM_LDFLAGS) \
	-lGEOMClient \
	-lSalomeIDLGEOM

SMESHEngine_CPPFLAGS = \
	$(libSMESHEngine_la_CPPFLAGS)

SMESHEngine_LDADD = \
	$(libSMESHEngine_la_LDFLAGS)
