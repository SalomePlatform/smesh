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
  ${KERNEL_INCLUDE_DIRS}
  ${GEOM_INCLUDE_DIRS}
  ${CAS_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${HDF5_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/src/Controls
  ${PROJECT_SOURCE_DIR}/src/MEDWrapper/Base
  ${PROJECT_SOURCE_DIR}/src/Driver
  ${PROJECT_SOURCE_DIR}/src/DriverDAT
  ${PROJECT_SOURCE_DIR}/src/DriverMED
  ${PROJECT_SOURCE_DIR}/src/DriverUNV
  ${PROJECT_SOURCE_DIR}/src/DriverSTL
  ${PROJECT_SOURCE_DIR}/src/DriverCGNS
  ${PROJECT_SOURCE_DIR}/src/DriverGMF
  ${PROJECT_SOURCE_DIR}/src/SMDS
  ${PROJECT_SOURCE_DIR}/src/SMESHDS
  ${PROJECT_SOURCE_DIR}/src/SMESHUtils
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${CAS_DEFINITIONS}
  ${BOOST_DEFINITIONS}
)

# optional sources
IF(SALOME_SMESH_USE_CGNS)
  SET(DriverCGNS_LIB MeshDriverCGNS)
ENDIF(SALOME_SMESH_USE_CGNS)

# libraries to link to
SET(_link_LIBRARIES
  ${CAS_TKShHealing}
  ${CAS_TKPrim}
  ${CAS_TKG2d}
  ${CAS_TKCDF}
  ${GEOM_NMTTools}
  ${GEOM_GEOMUtils}
  ${Boost_LIBRARIES}
  SMESHDS
  SMESHControls
  MeshDriverDAT
  MeshDriverSTL
  MeshDriverMED
  MeshDriverUNV
  MeshDriverGMF
  ${DriverCGNS_LIB}
)

# --- headers ---

# header files / no moc processing
SET(SMESHimpl_HEADERS
  SMESH_Gen.hxx
  SMESH_Mesh.hxx
  SMESH_subMesh.hxx
  SMESH_subMeshEventListener.hxx
  SMESH_Hypothesis.hxx
  SMESH_HypoFilter.hxx
  SMESH_Algo.hxx
  SMESH_Group.hxx
  SMESH_MeshEditor.hxx
  SMESH_Pattern.hxx
  SMESH_MesherHelper.hxx
  SMESH_ProxyMesh.hxx
  SMESH_SMESH.hxx
)

# --- sources ---

# sources / static
SET(SMESHimpl_SOURCES
  memoire.h
  SMESH_Gen.cxx
  SMESH_Mesh.cxx
  SMESH_subMesh.cxx
  SMESH_Hypothesis.cxx
  SMESH_Algo.cxx
  SMESH_Group.cxx
  SMESH_MeshEditor.cxx
  SMESH_Pattern.cxx
  SMESH_HypoFilter.cxx
  SMESH_ProxyMesh.cxx
  SMESH_MesherHelper.cxx
)

# --- rules ---

ADD_LIBRARY(SMESHimpl ${SMESHimpl_SOURCES})
TARGET_LINK_LIBRARIES(SMESHimpl ${_link_LIBRARIES} )
INSTALL(TARGETS SMESHimpl EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${SMESHimpl_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
