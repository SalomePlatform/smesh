# Copyright (C) 2012-2016  CEA/DEN, EDF R&D, OPEN CASCADE
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.
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

# --- options ---
# additional include directories

IF(SALOME_SMESH_USE_TBB)
  SET(TBB_INCLUDES ${TBB_INCLUDE_DIRS})
ENDIF(SALOME_SMESH_USE_TBB)

INCLUDE_DIRECTORIES(
  ${CAS_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${KERNEL_INCLUDE_DIRS}
  ${GEOM_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/src/SMESHUtils
  ${PROJECT_SOURCE_DIR}/src/SMESH
  ${PROJECT_SOURCE_DIR}/src/SMESHDS
  ${PROJECT_SOURCE_DIR}/src/SMDS
  ${PROJECT_SOURCE_DIR}/src/Controls
  ${TBB_INCLUDES}
)

IF(SALOME_SMESH_ENABLE_MEFISTO)
  INCLUDE_DIRECTORIES(
    ${PROJECT_SOURCE_DIR}/src/MEFISTO2
  )
ENDIF(SALOME_SMESH_ENABLE_MEFISTO)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${CAS_DEFINITIONS}
  ${BOOST_DEFINITIONS}
)

IF(SALOME_SMESH_USE_TBB)
  SET(TBB_LIBS ${TBB_LIBRARIES})
ENDIF(SALOME_SMESH_USE_TBB)

# libraries to link to
SET(_link_LIBRARIES
  ${KERNEL_SALOMELocalTrace}
  ${KERNEL_OpUtil}
  ${CAS_TKAdvTools}
  ${CAS_TKTopAlgo}
  ${CAS_TKG3d}
  ${CAS_TKOffset}
  ${GEOM_GEOMUtils}
  SMESHimpl
  SMESHDS
  ${TBB_LIBS}
)

IF(SALOME_SMESH_ENABLE_MEFISTO)
  SET(_link_LIBRARIES ${_link_LIBRARIES} MEFISTO2D)
ENDIF(SALOME_SMESH_ENABLE_MEFISTO)

# --- headers ---

# header files / no moc processing
SET(StdMeshers_HEADERS
  StdMeshers_LocalLength.hxx
  StdMeshers_Reversible1D.hxx
  StdMeshers_StartEndLength.hxx
  StdMeshers_Arithmetic1D.hxx
  StdMeshers_Geometric1D.hxx
  StdMeshers_FixedPoints1D.hxx
  StdMeshers_NumberOfSegments.hxx
  StdMeshers_Deflection1D.hxx
  StdMeshers_Propagation.hxx
  StdMeshers_LengthFromEdges.hxx
  StdMeshers_MaxElementArea.hxx
  StdMeshers_MaxElementVolume.hxx
  StdMeshers_NotConformAllowed.hxx
  StdMeshers_Regular_1D.hxx
  StdMeshers_Quadrangle_2D.hxx
  StdMeshers_Hexa_3D.hxx
  StdMeshers_Penta_3D.hxx
  StdMeshers_AutomaticLength.hxx
  StdMeshers_Distribution.hxx
  StdMeshers_QuadranglePreference.hxx
  StdMeshers_QuadraticMesh.hxx
  StdMeshers_NumberOfLayers.hxx
  StdMeshers_NumberOfLayers2D.hxx
  StdMeshers_Prism_3D.hxx
  StdMeshers_ProjectionSource1D.hxx
  StdMeshers_ProjectionSource2D.hxx
  StdMeshers_ProjectionSource3D.hxx
  StdMeshers_Projection_1D.hxx
  StdMeshers_Projection_2D.hxx
  StdMeshers_Projection_3D.hxx
  StdMeshers_RadialPrism_3D.hxx
  StdMeshers_ProjectionUtils.hxx
  StdMeshers_LayerDistribution.hxx
  StdMeshers_LayerDistribution2D.hxx
  StdMeshers_SegmentAroundVertex_0D.hxx
  StdMeshers_SegmentLengthAroundVertex.hxx
  StdMeshers_FaceSide.hxx
  StdMeshers_CompositeSegment_1D.hxx
  StdMeshers_UseExisting_1D2D.hxx
  StdMeshers_QuadToTriaAdaptor.hxx
  SMESH_StdMeshers.hxx
  StdMeshers_CompositeHexa_3D.hxx
  StdMeshers_MaxLength.hxx
  StdMeshers_QuadrangleParams.hxx
  StdMeshers_RadialQuadrangle_1D2D.hxx
  StdMeshers_HexaFromSkin_3D.hxx
  StdMeshers_ImportSource.hxx
  StdMeshers_Import_1D.hxx
  StdMeshers_Import_1D2D.hxx
  StdMeshers_ViscousLayers.hxx
  StdMeshers_ViscousLayers2D.hxx
  StdMeshers_Projection_1D2D.hxx
  StdMeshers_CartesianParameters3D.hxx
  StdMeshers_Cartesian_3D.hxx
  StdMeshers_QuadFromMedialAxis_1D2D.hxx
  StdMeshers_PolygonPerFace_2D.hxx
)

IF(SALOME_SMESH_ENABLE_MEFISTO)
  SET(StdMeshers_HEADERS ${StdMeshers_HEADERS} StdMeshers_MEFISTO_2D.hxx)
ENDIF(SALOME_SMESH_ENABLE_MEFISTO)
# --- sources ---

# sources / static
SET(StdMeshers_SOURCES
  StdMeshers_LocalLength.cxx
  StdMeshers_Reversible1D.cxx
  StdMeshers_StartEndLength.cxx
  StdMeshers_Arithmetic1D.cxx
  StdMeshers_Geometric1D.cxx
  StdMeshers_FixedPoints1D.cxx
  StdMeshers_NumberOfSegments.cxx
  StdMeshers_Deflection1D.cxx
  StdMeshers_Propagation.cxx
  StdMeshers_LengthFromEdges.cxx
  StdMeshers_MaxElementArea.cxx
  StdMeshers_MaxElementVolume.cxx
  StdMeshers_NotConformAllowed.cxx
  StdMeshers_Regular_1D.cxx
  StdMeshers_Quadrangle_2D.cxx
  StdMeshers_Penta_3D.cxx
  StdMeshers_Hexa_3D.cxx
  StdMeshers_AutomaticLength.cxx
  StdMeshers_Distribution.cxx
  StdMeshers_QuadranglePreference.cxx
  StdMeshers_QuadraticMesh.cxx
  StdMeshers_NumberOfLayers.cxx
  StdMeshers_NumberOfLayers2D.cxx
  StdMeshers_Prism_3D.cxx
  StdMeshers_ProjectionSource1D.cxx
  StdMeshers_ProjectionSource2D.cxx
  StdMeshers_ProjectionSource3D.cxx
  StdMeshers_Projection_1D.cxx
  StdMeshers_Projection_2D.cxx
  StdMeshers_Projection_3D.cxx
  StdMeshers_RadialPrism_3D.cxx
  StdMeshers_ProjectionUtils.cxx
  StdMeshers_LayerDistribution.cxx
  StdMeshers_LayerDistribution2D.cxx
  StdMeshers_SegmentAroundVertex_0D.cxx
  StdMeshers_SegmentLengthAroundVertex.cxx
  StdMeshers_FaceSide.cxx
  StdMeshers_CompositeSegment_1D.cxx
  StdMeshers_UseExisting_1D2D.cxx
  StdMeshers_QuadToTriaAdaptor.cxx
  StdMeshers_CompositeHexa_3D.cxx
  StdMeshers_MaxLength.cxx
  StdMeshers_QuadrangleParams.cxx
  StdMeshers_RadialQuadrangle_1D2D.cxx
  StdMeshers_HexaFromSkin_3D.cxx
  StdMeshers_ImportSource.cxx
  StdMeshers_Import_1D.cxx
  StdMeshers_Import_1D2D.cxx
  StdMeshers_ViscousLayers.cxx
  StdMeshers_ViscousLayers2D.cxx
  StdMeshers_Projection_1D2D.cxx
  StdMeshers_CartesianParameters3D.cxx
  StdMeshers_Cartesian_3D.cxx
  StdMeshers_Adaptive1D.cxx
  StdMeshers_QuadFromMedialAxis_1D2D.cxx
  StdMeshers_PolygonPerFace_2D.cxx
)

IF(SALOME_SMESH_ENABLE_MEFISTO)
  SET(StdMeshers_SOURCES ${StdMeshers_SOURCES} StdMeshers_MEFISTO_2D.cxx)
ENDIF(SALOME_SMESH_ENABLE_MEFISTO)

# --- rules ---

ADD_LIBRARY(StdMeshers ${StdMeshers_SOURCES})
TARGET_LINK_LIBRARIES(StdMeshers ${_link_LIBRARIES} )
INSTALL(TARGETS StdMeshers EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${StdMeshers_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
