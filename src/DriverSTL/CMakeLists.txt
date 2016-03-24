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
  ${CAS_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/src/Driver
  ${PROJECT_SOURCE_DIR}/src/SMDS
  ${PROJECT_SOURCE_DIR}/src/SMESHUtils
  ${PROJECT_SOURCE_DIR}/src/SMESH
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${CAS_DEFINITIONS}
  ${BOOST_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${CAS_TKernel}
  ${CAS_TKSTL}
  ${CAS_TKTopAlgo}
  ${CAS_TKMesh}
  ${KERNEL_SALOMEBasics}
  MeshDriver
  SMDS
  SMESHUtils
)

SET(_link_LIBRARIES_bin
  ${KERNEL_OpUtil}
  ${KERNEL_SALOMELocalTrace}
  ${KERNEL_SALOMEBasics}
  ${KERNEL_SalomeIDLKERNEL}
  ${CAS_TKernel}
  ${CAS_TKSTL}
  ${CAS_TKTopAlgo}
  ${CAS_TKMesh}
  ${CAS_TKBO}
  MeshDriver
  SMDS
)

# --- headers ---

# header files / no moc processing
SET(MeshDriverSTL_HEADERS
  DriverSTL_R_SMDS_Mesh.h
  DriverSTL_W_SMDS_Mesh.h
  SMESH_DriverSTL.hxx
)

# --- sources ---

# sources / static
SET(MeshDriverSTL_SOURCES
  DriverSTL_R_SMDS_Mesh.cxx
  DriverSTL_W_SMDS_Mesh.cxx
)

# --- rules ---

ADD_LIBRARY(MeshDriverSTL ${MeshDriverSTL_SOURCES})
TARGET_LINK_LIBRARIES(MeshDriverSTL ${_link_LIBRARIES} )
INSTALL(TARGETS MeshDriverSTL EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${MeshDriverSTL_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
