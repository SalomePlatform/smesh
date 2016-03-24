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
  ${MEDFILE_INCLUDE_DIRS}
  ${GEOM_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${CAS_INCLUDE_DIRS}
  ${OMNIORB_INCLUDE_DIR}
  ${PROJECT_SOURCE_DIR}/src/Controls
  ${PROJECT_SOURCE_DIR}/src/Driver
  ${PROJECT_SOURCE_DIR}/src/DriverDAT
  ${PROJECT_SOURCE_DIR}/src/DriverMED
  ${PROJECT_SOURCE_DIR}/src/DriverUNV
  ${PROJECT_SOURCE_DIR}/src/DriverSTL
  ${PROJECT_SOURCE_DIR}/src/SMDS
  ${PROJECT_SOURCE_DIR}/src/SMESHDS
  ${PROJECT_SOURCE_DIR}/src/SMESH
  ${PROJECT_SOURCE_DIR}/src/SMESHUtils
  ${PROJECT_BINARY_DIR}/idl
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${OMNIORB_DEFINITIONS}
  ${CAS_DEFINITIONS}
  ${BOOST_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${KERNEL_SalomeLifeCycleCORBA}
  ${KERNEL_SalomeNS}
  ${KERNEL_OpUtil}
  ${KERNEL_SALOMEBasics}
  ${KERNEL_SalomeContainer}
  ${KERNEL_SalomeHDFPersist}
  ${KERNEL_SalomeGenericObj}
  ${KERNEL_Registry}
  ${KERNEL_ResourcesManager}
  ${KERNEL_SalomeResourcesManager}
  ${KERNEL_SalomeNotification}
  ${KERNEL_SalomeIDLKERNEL}
  ${KERNEL_SALOMELocalTrace}
  ${GEOM_NMTDS}
  ${GEOM_SalomeIDLGEOM}  
  ${CAS_KERNEL}
  SalomeIDLSMESH
  SMDS
  SMESHimpl
  SMESHDS
  SMESHControls
  MEDWrapper
  MEDWrapperBase
  MEDWrapper_V2_2
)

# --- headers ---

# header files / no moc processing
SET(SMESHClient_HEADERS
  SMESH_Client.hxx
)

# --- sources ---

# sources / static
SET(SMESHClient_SOURCES
  SMESH_Client.cxx
)

# --- rules ---

ADD_LIBRARY(SMESHClient ${SMESHClient_SOURCES})
TARGET_LINK_LIBRARIES(SMESHClient ${_link_LIBRARIES} )
INSTALL(TARGETS SMESHClient EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})


INSTALL(FILES ${SMESHClient_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
