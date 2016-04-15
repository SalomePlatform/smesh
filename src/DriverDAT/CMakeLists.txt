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
  ${PROJECT_SOURCE_DIR}/src/SMESHDS
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${CAS_DEFINITIONS}
  ${BOOST_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${CAS_KERNEL}
  ${KERNEL_SALOMEBasics}
  ${KERNEL_SalomeIDLKERNEL}
  MeshDriver
)

SET(_link_LIBRARIES_bin
  ${KERNEL_OpUtil}
  ${KERNEL_SALOMELocalTrace}
  ${KERNEL_SALOMEBasics}
  ${KERNEL_SalomeIDLKERNEL}
  MeshDriver
  SMDS
)

# --- headers ---

# header files / no moc processing
SET(MeshDriverDAT_HEADERS
  DriverDAT_R_SMDS_Mesh.h
  DriverDAT_W_SMDS_Mesh.h
  SMESH_DriverDAT.hxx
)

# --- sources ---

# sources / static
SET(MeshDriverDAT_SOURCES
  DriverDAT_R_SMDS_Mesh.cxx
  DriverDAT_W_SMDS_Mesh.cxx
)

# --- rules ---

ADD_LIBRARY(MeshDriverDAT ${MeshDriverDAT_SOURCES})
TARGET_LINK_LIBRARIES(MeshDriverDAT ${_link_LIBRARIES} )
INSTALL(TARGETS MeshDriverDAT EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${MeshDriverDAT_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
