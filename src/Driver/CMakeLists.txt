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
  ${Boost_INCLUDE_DIRS}
  ${CAS_INCLUDE_DIRS}
  ${KERNEL_INCLUDE_DIRS}
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
  ${KERNEL_OpUtil}
  ${CAS_TKernel}
  ${CAS_TKTopAlgo}
  SMESHDS
  SMESHUtils
)

# --- headers ---

# header files / no moc processing
SET(MeshDriver_HEADERS
  Driver_Document.h
  Driver_Mesh.h
  Driver_SMDS_Mesh.h
  Driver_SMESHDS_Mesh.h
)

# --- sources ---

# sources / static
SET(MeshDriver_SOURCES
  Driver_Document.cxx
  Driver_Mesh.cxx
  Driver_SMDS_Mesh.cxx
  Driver_SMESHDS_Mesh.cxx 
)

# --- rules ---

ADD_LIBRARY(MeshDriver ${MeshDriver_SOURCES})
TARGET_LINK_LIBRARIES(MeshDriver ${_link_LIBRARIES} )
INSTALL(TARGETS MeshDriver EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${MeshDriver_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
