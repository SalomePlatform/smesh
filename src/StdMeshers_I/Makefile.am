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

#  SMESH StdMeshers_I : idl implementation based on 'StdMeshersPlugin' unit's classes
#  File   : Makefile.in
#  Author : Julia DOROVSKIKH
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : SMESH
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
	StdMeshers_LocalLength_i.hxx \
	StdMeshers_StartEndLength_i.hxx \
	StdMeshers_Arithmetic1D_i.hxx \
	StdMeshers_FixedPoints1D_i.hxx \
	StdMeshers_NumberOfSegments_i.hxx \
	StdMeshers_Deflection1D_i.hxx \
	StdMeshers_Propagation_i.hxx \
	StdMeshers_LengthFromEdges_i.hxx \
	StdMeshers_MaxElementArea_i.hxx \
	StdMeshers_MaxElementVolume_i.hxx \
	StdMeshers_NotConformAllowed_i.hxx \
	StdMeshers_Regular_1D_i.hxx \
	StdMeshers_Quadrangle_2D_i.hxx \
	StdMeshers_MEFISTO_2D_i.hxx \
	StdMeshers_Hexa_3D_i.hxx \
	StdMeshers_AutomaticLength_i.hxx \
	StdMeshers_QuadranglePreference_i.hxx \
	StdMeshers_QuadraticMesh_i.hxx \
	StdMeshers_NumberOfLayers_i.hxx \
	StdMeshers_NumberOfLayers2D_i.hxx \
	StdMeshers_Prism_3D_i.hxx \
	StdMeshers_ProjectionSource1D_i.hxx \
	StdMeshers_ProjectionSource2D_i.hxx \
	StdMeshers_ProjectionSource3D_i.hxx \
	StdMeshers_Projection_1D_2D_3D_i.hxx \
	StdMeshers_ObjRefUlils.hxx \
	StdMeshers_LayerDistribution_i.hxx \
	StdMeshers_LayerDistribution2D_i.hxx \
	StdMeshers_CompositeSegment_1D_i.hxx \
	StdMeshers_SegmentAroundVertex_0D_i.hxx \
	StdMeshers_SegmentLengthAroundVertex_i.hxx \
	StdMeshers_UseExisting_1D2D_i.hxx \
	StdMeshers_MaxLength_i.hxx \
	StdMeshers_QuadrangleParams_i.hxx \
	StdMeshers_RadialQuadrangle_1D2D_i.hxx \
	SMESH_StdMeshers_I.hxx \
	StdMeshers_ImportSource1D_i.hxx \
	StdMeshers_ImportSource2D_i.hxx \
	StdMeshers_Import_1D_i.hxx \
	StdMeshers_Import_1D2D_i.hxx \
	StdMeshers_ViscousLayers_i.hxx \
	StdMeshers_CartesianParameters3D_i.hxx \
	StdMeshers_Cartesian_3D_i.hxx

# Libraries targets
lib_LTLIBRARIES = libStdMeshersEngine.la

dist_libStdMeshersEngine_la_SOURCES = \
	StdMeshers_i.cxx \
        StdMeshers_LocalLength_i.cxx \
	StdMeshers_StartEndLength_i.cxx \
	StdMeshers_Arithmetic1D_i.cxx \
	StdMeshers_FixedPoints1D_i.cxx \
	StdMeshers_NumberOfSegments_i.cxx \
	StdMeshers_Deflection1D_i.cxx \
        StdMeshers_Propagation_i.cxx \
        StdMeshers_LengthFromEdges_i.cxx \
	StdMeshers_MaxElementArea_i.cxx \
        StdMeshers_MaxElementVolume_i.cxx \
	StdMeshers_NotConformAllowed_i.cxx \
        StdMeshers_Regular_1D_i.cxx \
        StdMeshers_Quadrangle_2D_i.cxx \
        StdMeshers_MEFISTO_2D_i.cxx \
	StdMeshers_Hexa_3D_i.cxx \
	StdMeshers_AutomaticLength_i.cxx \
	StdMeshers_QuadranglePreference_i.cxx \
	StdMeshers_QuadraticMesh_i.cxx \
	StdMeshers_NumberOfLayers_i.cxx \
	StdMeshers_NumberOfLayers2D_i.cxx \
	StdMeshers_Prism_3D_i.cxx \
	StdMeshers_ProjectionSource1D_i.cxx \
	StdMeshers_ProjectionSource2D_i.cxx \
	StdMeshers_ProjectionSource3D_i.cxx \
	StdMeshers_Projection_1D_2D_3D_i.cxx \
	StdMeshers_ObjRefUlils.cxx \
	StdMeshers_LayerDistribution_i.cxx \
	StdMeshers_LayerDistribution2D_i.cxx \
	StdMeshers_CompositeSegment_1D_i.cxx \
	StdMeshers_SegmentAroundVertex_0D_i.cxx \
	StdMeshers_SegmentLengthAroundVertex_i.cxx \
	StdMeshers_UseExisting_1D2D_i.cxx \
	StdMeshers_MaxLength_i.cxx \
	StdMeshers_QuadrangleParams_i.cxx \
	StdMeshers_RadialQuadrangle_1D2D_i.cxx \
	StdMeshers_ImportSource1D_i.cxx \
	StdMeshers_ImportSource2D_i.cxx \
	StdMeshers_Import_1D_i.cxx \
	StdMeshers_Import_1D2D_i.cxx \
	StdMeshers_ViscousLayers_i.cxx \
	StdMeshers_CartesianParameters3D_i.cxx \
	StdMeshers_Cartesian_3D_i.cxx

# additionnal information to compil and link file
libStdMeshersEngine_la_CPPFLAGS = \
	$(CAS_CPPFLAGS) \
	@HDF5_INCLUDES@ \
	$(KERNEL_CXXFLAGS) \
	$(GEOM_CXXFLAGS) \
	$(MED_CXXFLAGS) \
	$(BOOST_CPPFLAGS) \
        $(VTK_INCLUDES) \
	$(CORBA_CXXFLAGS) \
	$(CORBA_INCLUDES) \
	-I$(srcdir)/../SMESHImpl \
	-I$(srcdir)/../MEFISTO2 \
	-I$(srcdir)/../SMESH \
	-I$(srcdir)/../SMESHUtils \
	-I$(srcdir)/../SMESH_I \
	-I$(srcdir)/../SMESHDS \
	-I$(srcdir)/../SMDS \
	-I$(srcdir)/../Controls \
	-I$(srcdir)/../StdMeshers \
	-I$(top_builddir)/idl

libStdMeshersEngine_la_LDFLAGS  = \
	../../idl/libSalomeIDLSMESH.la \
	../StdMeshers/libStdMeshers.la \
	../SMESH_I/libSMESHEngine.la \
	../SMESH/libSMESHimpl.la \
	../SMESHDS/libSMESHDS.la \
	$(KERNEL_LDFLAGS) -lSALOMELocalTrace -lSalomeGenericObj \
	@HDF5_LIBS@ \
	$(CAS_LDPATH) -lTKBRep -lTKG3d
