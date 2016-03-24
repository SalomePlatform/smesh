# Copyright (C) 2012-2016  EDF R&D
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
  ${VTK_INCLUDE_DIRS}
  ${CAS_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${OMNIORB_INCLUDE_DIR}
  ${LIBXML2_INCLUDE_DIR}
  ${PROJECT_SOURCE_DIR}/src/SMESH
  ${PROJECT_SOURCE_DIR}/src/SMESH_I
  ${PROJECT_SOURCE_DIR}/src/SMESHDS
  ${PROJECT_SOURCE_DIR}/src/SMDS
  ${PROJECT_SOURCE_DIR}/src/SMESHUtils
  ${PROJECT_BINARY_DIR}/src/Tools/padder/meshjob/idl
  ${PROJECT_BINARY_DIR}/idl
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${CAS_DEFINITIONS}
  ${BOOST_DEFINITIONS}
  ${LIBXML2_DEFINITIONS}
  ${OMNIORB_DEFINITIONS}
)

IF(WIN32)
  ADD_DEFINITIONS(-DNOGDI)
ENDIF(WIN32)

# libraries to link to
SET(MeshJobManagerEngine_LIBRARIES
  ${KERNEL_SalomeContainer}
  ${KERNEL_SalomeKernelHelpers}
  ${KERNEL_SalomeLifeCycleCORBA}
  SalomeIDLSPADDER
)

SET(SPADDERPluginTesterEngine_LIBRARIES
  ${KERNEL_SalomeContainer}
  ${KERNEL_SalomeKernelHelpers}
  SMESHimpl
  SMESHEngine
  SMESHDS
  SMDS
  SMESHUtils
  SalomeIDLSPADDER
)

# --- headers ---

# header files / no moc processing
SET(MeshJobManagerEngine_HEADERS
  MeshJobManager_i.hxx
)

# --- sources ---

# sources / static
SET(MeshJobManagerEngine_SOURCES
  MeshJobManager_i.cxx 
)

SET(SPADDERPluginTesterEngine_SOURCES
  SPADDERPluginTester_i.hxx
  SPADDERPluginTester_i.cxx
)

# --- rules ---

ADD_LIBRARY(MeshJobManagerEngine ${MeshJobManagerEngine_SOURCES})
TARGET_LINK_LIBRARIES(MeshJobManagerEngine ${MeshJobManagerEngine_LIBRARIES} )
INSTALL(TARGETS MeshJobManagerEngine EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

ADD_LIBRARY(SPADDERPluginTesterEngine ${SPADDERPluginTesterEngine_SOURCES})
TARGET_LINK_LIBRARIES(SPADDERPluginTesterEngine ${SPADDERPluginTesterEngine_LIBRARIES} )
INSTALL(TARGETS SPADDERPluginTesterEngine EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})


INSTALL(FILES ${MeshJobManagerEngine_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
