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
INCLUDE_DIRECTORIES(
  ${CAS_INCLUDE_DIRS}
  ${HDF5_INCLUDE_DIRS}
  ${KERNEL_INCLUDE_DIRS}
  ${GEOM_INCLUDE_DIRS}
  ${MEDFILE_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${OMNIORB_INCLUDE_DIR}
  ${PROJECT_SOURCE_DIR}/src/SMESHImpl
  ${PROJECT_SOURCE_DIR}/src/SMESH
  ${PROJECT_SOURCE_DIR}/src/SMESHUtils
  ${PROJECT_SOURCE_DIR}/src/SMESH_I
  ${PROJECT_SOURCE_DIR}/src/SMESHDS
  ${PROJECT_SOURCE_DIR}/src/SMDS
  ${PROJECT_SOURCE_DIR}/src/Controls
  ${PROJECT_SOURCE_DIR}/src/StdMeshers
  ${PROJECT_BINARY_DIR}/idl
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
  ${OMNIORB_DEFINITIONS}
)

# To avoid GetObject macro definition on WIN32
IF(WIN32)
  ADD_DEFINITIONS(-DNOGDI)
ENDIF(WIN32)

# libraries to link to
SET(_link_LIBRARIES
  ${KERNEL_SALOMELocalTrace}
  ${KERNEL_SalomeGenericObj}
  ${CAS_TKBRep}
  ${CAS_TKG3d}
  SalomeIDLSMESH
  StdMeshers
  SMESHEngine
  SMESHimpl
  SMESHDS
  ${HDF5_LIBRARIES}

)

# --- headers ---

# header files / no moc processing
SET(StdMeshersEngine_HEADERS
  StdMeshers_LocalLength_i.hxx
  StdMeshers_StartEndLength_i.hxx
  StdMeshers_Arithmetic1D_i.hxx
  StdMeshers_FixedPoints1D_i.hxx
  StdMeshers_NumberOfSegments_i.hxx
  StdMeshers_Deflection1D_i.hxx
  StdMeshers_Propagation_i.hxx
  StdMeshers_LengthFromEdges_i.hxx
  StdMeshers_MaxElementArea_i.hxx
  StdMeshers_MaxElementVolume_i.hxx
  StdMeshers_NotConformAllowed_i.hxx
  StdMeshers_Regular_1D_i.hxx
  StdMeshers_Quadrangle_2D_i.hxx
  StdMeshers_Hexa_3D_i.hxx
  StdMeshers_AutomaticLength_i.hxx
  StdMeshers_QuadranglePreference_i.hxx
  StdMeshers_QuadraticMesh_i.hxx
  StdMeshers_NumberOfLayers_i.hxx
  StdMeshers_NumberOfLayers2D_i.hxx
  StdMeshers_Prism_3D_i.hxx
  StdMeshers_ProjectionSource1D_i.hxx
  StdMeshers_ProjectionSource2D_i.hxx
  StdMeshers_ProjectionSource3D_i.hxx
  StdMeshers_Projection_1D_2D_3D_i.hxx
  StdMeshers_ObjRefUlils.hxx
  StdMeshers_LayerDistribution_i.hxx
  StdMeshers_LayerDistribution2D_i.hxx
  StdMeshers_CompositeSegment_1D_i.hxx
  StdMeshers_SegmentAroundVertex_0D_i.hxx
  StdMeshers_SegmentLengthAroundVertex_i.hxx
  StdMeshers_UseExisting_1D2D_i.hxx
  StdMeshers_MaxLength_i.hxx
  StdMeshers_QuadrangleParams_i.hxx
  StdMeshers_RadialQuadrangle_1D2D_i.hxx
  SMESH_StdMeshers_I.hxx
  StdMeshers_ImportSource1D_i.hxx
  StdMeshers_ImportSource2D_i.hxx
  StdMeshers_Import_1D_i.hxx
  StdMeshers_Import_1D2D_i.hxx
  StdMeshers_ViscousLayers_i.hxx
  StdMeshers_ViscousLayers2D_i.hxx
  StdMeshers_CartesianParameters3D_i.hxx
  StdMeshers_Cartesian_3D_i.hxx
  StdMeshers_PolygonPerFace_2D_i.hxx
)
IF(SALOME_SMESH_ENABLE_MEFISTO)
  SET(StdMeshersEngine_HEADERS ${StdMeshersEngine_HEADERS} StdMeshers_MEFISTO_2D_i.hxx)
ENDIF(SALOME_SMESH_ENABLE_MEFISTO)
# --- sources ---

# sources / static
SET(StdMeshersEngine_SOURCES
  StdMeshers_i.cxx
  StdMeshers_LocalLength_i.cxx 
  StdMeshers_Reversible1D_i.cxx
  StdMeshers_StartEndLength_i.cxx
  StdMeshers_Arithmetic1D_i.cxx
  StdMeshers_Geometric1D_i.cxx
  StdMeshers_FixedPoints1D_i.cxx
  StdMeshers_NumberOfSegments_i.cxx
  StdMeshers_Deflection1D_i.cxx
  StdMeshers_Propagation_i.cxx
  StdMeshers_LengthFromEdges_i.cxx
  StdMeshers_MaxElementArea_i.cxx
  StdMeshers_MaxElementVolume_i.cxx
  StdMeshers_NotConformAllowed_i.cxx
  StdMeshers_Regular_1D_i.cxx
  StdMeshers_Quadrangle_2D_i.cxx  
  StdMeshers_Hexa_3D_i.cxx
  StdMeshers_AutomaticLength_i.cxx
  StdMeshers_QuadranglePreference_i.cxx
  StdMeshers_QuadraticMesh_i.cxx
  StdMeshers_NumberOfLayers_i.cxx
  StdMeshers_NumberOfLayers2D_i.cxx
  StdMeshers_Prism_3D_i.cxx
  StdMeshers_ProjectionSource1D_i.cxx
  StdMeshers_ProjectionSource2D_i.cxx
  StdMeshers_ProjectionSource3D_i.cxx
  StdMeshers_Projection_1D_2D_3D_i.cxx
  StdMeshers_ObjRefUlils.cxx
  StdMeshers_LayerDistribution_i.cxx
  StdMeshers_LayerDistribution2D_i.cxx
  StdMeshers_CompositeSegment_1D_i.cxx
  StdMeshers_SegmentAroundVertex_0D_i.cxx
  StdMeshers_SegmentLengthAroundVertex_i.cxx
  StdMeshers_UseExisting_1D2D_i.cxx
  StdMeshers_MaxLength_i.cxx
  StdMeshers_QuadrangleParams_i.cxx
  StdMeshers_RadialQuadrangle_1D2D_i.cxx
  StdMeshers_ImportSource1D_i.cxx
  StdMeshers_ImportSource2D_i.cxx
  StdMeshers_Import_1D_i.cxx
  StdMeshers_Import_1D2D_i.cxx
  StdMeshers_ViscousLayers_i.cxx
  StdMeshers_ViscousLayers2D_i.cxx
  StdMeshers_CartesianParameters3D_i.cxx
  StdMeshers_Cartesian_3D_i.cxx
  StdMeshers_Adaptive1D_i.cxx 
  StdMeshers_PolygonPerFace_2D_i.cxx
)

IF(SALOME_SMESH_ENABLE_MEFISTO)
  SET(StdMeshersEngine_SOURCES ${StdMeshersEngine_SOURCES} StdMeshers_MEFISTO_2D_i.cxx)
ENDIF(SALOME_SMESH_ENABLE_MEFISTO)

# --- rules ---

ADD_LIBRARY(StdMeshersEngine ${StdMeshersEngine_SOURCES})
TARGET_LINK_LIBRARIES(StdMeshersEngine ${_link_LIBRARIES} )
INSTALL(TARGETS StdMeshersEngine EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${StdMeshersEngine_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
