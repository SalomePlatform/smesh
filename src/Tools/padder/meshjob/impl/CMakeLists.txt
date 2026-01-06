# Copyright (C) 2012-2026  CEA, EDF
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

include(${SWIG_USE_FILE})

# --- options ---
INCLUDE_DIRECTORIES(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${KERNEL_INCLUDE_DIRS}
  ${GEOM_INCLUDE_DIRS}
  ${OpenCASCADE_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${OMNIORB_INCLUDE_DIR}
  ${LIBXML2_INCLUDE_DIR}
  ${MEDCOUPLING_INCLUDE_DIRS}
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
  ${OpenCASCADE_DEFINITIONS}
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

# - swiggy pop

SET(SMeshPadderHelper_HEADERS SMeshPadderHelper.h SMeshPadderHelper.i)
SET(SMeshPadderHelper_SOURCES SMeshPadderHelper.cxx ${SMeshPadderHelper_HEADERS})
SET_SOURCE_FILES_PROPERTIES(SMeshPadderHelper.i PROPERTIES CPLUSPLUS ON)
SET_SOURCE_FILES_PROPERTIES(SMeshPadderHelper.i PROPERTIES SWIG_FLAGS "-py3")
SET_SOURCE_FILES_PROPERTIES(SMeshPadderHelper_wrap.cpp PROPERTIES COMPILE_FLAGS "-DHAVE_CONFIG_H")
SET(_swig_SCRIPTS ${CMAKE_CURRENT_BINARY_DIR}/SMeshPadderHelper.py )
IF(${CMAKE_VERSION} VERSION_LESS "3.8.0") 
  SWIG_ADD_MODULE(SMeshPadderHelper python ${SMeshPadderHelper_SOURCES})
ELSE()
  SWIG_ADD_LIBRARY(SMeshPadderHelper LANGUAGE python SOURCES ${SMeshPadderHelper_SOURCES})
ENDIF()
SWIG_LINK_LIBRARIES(SMeshPadderHelper ${PYTHON_LIBRARIES} ${PLATFORM_LIBS} MeshJobManagerEngine )
SWIG_CHECK_GENERATION(SMeshPadderHelper)
IF(WIN32)
  SET_TARGET_PROPERTIES(_SMeshPadderHelper PROPERTIES DEBUG_OUTPUT_NAME _SMeshPadderHelper_d)
ENDIF(WIN32)
install(TARGETS _SMeshPadderHelper DESTINATION ${SALOME_INSTALL_LIBS})
install(FILES ${SMeshPadderHelper_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
SALOME_INSTALL_SCRIPTS("${_swig_SCRIPTS}"  ${SALOME_INSTALL_BINS} EXTRA_DPYS "${SWIG_MODULE_SMeshPadderHelper_REAL_NAME}")

# --- rules ---

ADD_LIBRARY(MeshJobManagerEngine ${MeshJobManagerEngine_SOURCES})
TARGET_LINK_LIBRARIES(MeshJobManagerEngine ${MeshJobManagerEngine_LIBRARIES} )
INSTALL(TARGETS MeshJobManagerEngine EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

ADD_LIBRARY(SPADDERPluginTesterEngine ${SPADDERPluginTesterEngine_SOURCES})
TARGET_LINK_LIBRARIES(SPADDERPluginTesterEngine ${SPADDERPluginTesterEngine_LIBRARIES} )
INSTALL(TARGETS SPADDERPluginTesterEngine EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})


INSTALL(FILES ${MeshJobManagerEngine_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
