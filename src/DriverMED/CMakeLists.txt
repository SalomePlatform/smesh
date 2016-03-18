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
  ${HDF5_INCLUDE_DIRS}
  ${KERNEL_INCLUDE_DIRS}
  ${CAS_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/src/MEDWrapper/Base
  ${PROJECT_SOURCE_DIR}/src/MEDWrapper/Factory
  ${PROJECT_SOURCE_DIR}/src/Driver
  ${PROJECT_SOURCE_DIR}/src/SMDS
  ${PROJECT_SOURCE_DIR}/src/SMESHUtils
  ${PROJECT_SOURCE_DIR}/src/SMESHDS
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${HDF5_DEFINITIONS}
  ${CAS_DEFINITIONS}
  ${BOOST_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${Boost_LIBRARIES}
  MeshDriver
  MEDWrapper
  MEDWrapperBase
  MEDWrapper_V2_2
)

SET(_link_LIBRARIES_bin
  ${_link_LIBRARIES}
  ${KERNEL_OpUtil}
  ${KERNEL_SALOMELocalTrace}
  ${KERNEL_SALOMEBasics}
  ${KERNEL_SalomeIDLKERNEL}
  SMDS
  SMESHDS
)

# --- headers ---

# header files / no moc processing
SET(MeshDriverMED_HEADERS
  DriverMED.hxx
  DriverMED_R_SMESHDS_Mesh.h
  DriverMED_W_SMESHDS_Mesh.h
  DriverMED_Family.h
  DriverMED_W_Field.h
  SMESH_DriverMED.hxx
)

# --- sources ---

# sources / static
SET(MeshDriverMED_SOURCES
  DriverMED_R_SMESHDS_Mesh.cxx
  DriverMED_W_SMESHDS_Mesh.cxx
  DriverMED_Family.cxx
  DriverMED_W_Field.cxx
)

# --- rules ---

ADD_LIBRARY(MeshDriverMED ${MeshDriverMED_SOURCES})
TARGET_LINK_LIBRARIES(MeshDriverMED ${_link_LIBRARIES} )
INSTALL(TARGETS MeshDriverMED EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${MeshDriverMED_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
