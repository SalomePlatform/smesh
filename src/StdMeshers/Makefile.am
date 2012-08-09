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

#  SMESH StdMeshers : implementaion of SMESH idl descriptions
#  File   : Makefile.in
#  Author : Julia DOROVSKIKH
#  Modified by : Alexander BORODIN (OCN) - autotools usage
#  Module : SMESH
#
include $(top_srcdir)/adm_local/unix/make_common_starter.am

# header files 
salomeinclude_HEADERS = \
	StdMeshers_LocalLength.hxx \
	StdMeshers_StartEndLength.hxx \
	StdMeshers_Arithmetic1D.hxx \
	StdMeshers_FixedPoints1D.hxx \
	StdMeshers_NumberOfSegments.hxx \
	StdMeshers_Deflection1D.hxx \
	StdMeshers_Propagation.hxx \
	StdMeshers_LengthFromEdges.hxx \
	StdMeshers_MaxElementArea.hxx \
	StdMeshers_MaxElementVolume.hxx \
	StdMeshers_NotConformAllowed.hxx \
	StdMeshers_Regular_1D.hxx \
	StdMeshers_Quadrangle_2D.hxx \
	StdMeshers_MEFISTO_2D.hxx \
	StdMeshers_Hexa_3D.hxx \
	StdMeshers_Penta_3D.hxx \
	StdMeshers_AutomaticLength.hxx \
	StdMeshers_Distribution.hxx \
	StdMeshers_QuadranglePreference.hxx \
	StdMeshers_QuadraticMesh.hxx \
	StdMeshers_NumberOfLayers.hxx \
	StdMeshers_NumberOfLayers2D.hxx \
	StdMeshers_Prism_3D.hxx \
	StdMeshers_ProjectionSource1D.hxx \
	StdMeshers_ProjectionSource2D.hxx \
	StdMeshers_ProjectionSource3D.hxx \
	StdMeshers_Projection_1D.hxx \
	StdMeshers_Projection_2D.hxx \
	StdMeshers_Projection_3D.hxx \
	StdMeshers_RadialPrism_3D.hxx  \
	StdMeshers_ProjectionUtils.hxx \
	StdMeshers_LayerDistribution.hxx \
	StdMeshers_LayerDistribution2D.hxx \
	StdMeshers_SegmentAroundVertex_0D.hxx \
	StdMeshers_SegmentLengthAroundVertex.hxx \
	StdMeshers_FaceSide.hxx \
	StdMeshers_CompositeSegment_1D.hxx \
	StdMeshers_UseExisting_1D2D.hxx \
	StdMeshers_QuadToTriaAdaptor.hxx \
	SMESH_StdMeshers.hxx \
	StdMeshers_CompositeHexa_3D.hxx \
	StdMeshers_MaxLength.hxx \
	StdMeshers_QuadrangleParams.hxx \
	StdMeshers_RadialQuadrangle_1D2D.hxx \
	StdMeshers_HexaFromSkin_3D.hxx \
	StdMeshers_ImportSource.hxx \
	StdMeshers_Import_1D.hxx \
	StdMeshers_Import_1D2D.hxx \
	StdMeshers_ViscousLayers.hxx \
	StdMeshers_Projection_1D2D.hxx \
	StdMeshers_CartesianParameters3D.hxx \
	StdMeshers_Cartesian_3D.hxx

# Libraries targets

lib_LTLIBRARIES = libStdMeshers.la

dist_libStdMeshers_la_SOURCES = \
	StdMeshers_LocalLength.cxx \
	StdMeshers_StartEndLength.cxx \
	StdMeshers_Arithmetic1D.cxx \
	StdMeshers_FixedPoints1D.cxx \
	StdMeshers_NumberOfSegments.cxx \
	StdMeshers_Deflection1D.cxx \
	StdMeshers_Propagation.cxx \
	StdMeshers_LengthFromEdges.cxx \
	StdMeshers_MaxElementArea.cxx \
	StdMeshers_MaxElementVolume.cxx \
	StdMeshers_NotConformAllowed.cxx \
	StdMeshers_Regular_1D.cxx \
	StdMeshers_Quadrangle_2D.cxx \
	StdMeshers_MEFISTO_2D.cxx \
	StdMeshers_Penta_3D.cxx \
	StdMeshers_Hexa_3D.cxx \
	StdMeshers_AutomaticLength.cxx \
	StdMeshers_Distribution.cxx \
	StdMeshers_QuadranglePreference.cxx \
	StdMeshers_QuadraticMesh.cxx \
	StdMeshers_NumberOfLayers.cxx \
	StdMeshers_NumberOfLayers2D.cxx \
	StdMeshers_Prism_3D.cxx \
	StdMeshers_ProjectionSource1D.cxx \
	StdMeshers_ProjectionSource2D.cxx \
	StdMeshers_ProjectionSource3D.cxx \
	StdMeshers_Projection_1D.cxx \
	StdMeshers_Projection_2D.cxx \
	StdMeshers_Projection_3D.cxx \
	StdMeshers_RadialPrism_3D.cxx \
	StdMeshers_ProjectionUtils.cxx \
	StdMeshers_LayerDistribution.cxx \
	StdMeshers_LayerDistribution2D.cxx \
	StdMeshers_SegmentAroundVertex_0D.cxx \
	StdMeshers_SegmentLengthAroundVertex.cxx \
	StdMeshers_FaceSide.cxx \
	StdMeshers_CompositeSegment_1D.cxx \
	StdMeshers_UseExisting_1D2D.cxx \
	StdMeshers_QuadToTriaAdaptor.cxx \
	StdMeshers_CompositeHexa_3D.cxx \
	StdMeshers_MaxLength.cxx \
	StdMeshers_QuadrangleParams.cxx \
	StdMeshers_RadialQuadrangle_1D2D.cxx \
	StdMeshers_HexaFromSkin_3D.cxx \
	StdMeshers_ImportSource.cxx \
	StdMeshers_Import_1D.cxx \
	StdMeshers_Import_1D2D.cxx \
	StdMeshers_ViscousLayers.cxx \
	StdMeshers_Projection_1D2D.cxx \
	StdMeshers_CartesianParameters3D.cxx \
	StdMeshers_Cartesian_3D.cxx

# additionnal information to compil and link file
libStdMeshers_la_CPPFLAGS = \
	$(CAS_CPPFLAGS) \
	$(BOOST_CPPFLAGS) \
        $(VTK_INCLUDES) \
	$(KERNEL_CXXFLAGS) \
	$(GUI_CXXFLAGS) \
	-I$(srcdir)/../MEFISTO2 \
	-I$(srcdir)/../SMESHUtils \
	-I$(srcdir)/../SMESH \
	-I$(srcdir)/../SMESHDS \
	-I$(srcdir)/../SMDS \
	-I$(srcdir)/../Controls \
	@PLATFORM_INCLUDES@

libStdMeshers_la_LDFLAGS  = \
	../SMESH/libSMESHimpl.la \
	../SMESHDS/libSMESHDS.la \
	../MEFISTO2/libMEFISTO2D.la \
	$(KERNEL_LDFLAGS) -lSALOMELocalTrace -lOpUtil \
	$(CAS_LDPATH) -lTKAdvTools -lTKTopAlgo -lTKG3d

if WITH_TBB
  libStdMeshers_la_CPPFLAGS += $(TBB_INCLUDES)
  libStdMeshers_la_LDFLAGS  += $(TBB_LIBS)
endif
