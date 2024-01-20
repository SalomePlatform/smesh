# Copyright (C) 2012-2024  CEA, EDF, OPEN CASCADE
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

include_directories(
  ${PROJECT_SOURCE_DIR}/src/SMESHDS
  ${PROJECT_SOURCE_DIR}/src/SMESHUtils
  ${PROJECT_SOURCE_DIR}/src/SMDS
  ${PROJECT_SOURCE_DIR}/src/SMESH
  ${PROJECT_SOURCE_DIR}/src/SMESH_I
  ${PROJECT_SOURCE_DIR}/src/Driver
  ${PROJECT_SOURCE_DIR}/src/DriverGMF
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_BINARY_DIR}/idl
)

# scripts / static

SET(smesh_SCRIPTS
  smeshBuilder.py
  smesh_algorithm.py
  smesh_selection.py
  smesh_tools.py
)

SET(smesh_exe_SCRIPTS
  mesher_launcher.py
  send_files.py
)

SET(StdMeshers_SCRIPTS
  __init__.py
  StdMeshersBuilder.py
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(${OMNIORB_DEFINITIONS})
SET(SMeshHelper_HEADERS SMeshHelper.h SMeshHelper.i)
SET(SMeshHelper_SOURCES SMeshHelper.cxx ${SMeshHelper_HEADERS})
SET_SOURCE_FILES_PROPERTIES(SMeshHelper.i PROPERTIES CPLUSPLUS ON)
SET_SOURCE_FILES_PROPERTIES(SMeshHelper.i PROPERTIES SWIG_FLAGS "-py3")
SET_SOURCE_FILES_PROPERTIES(SMeshHelper_wrap.cpp PROPERTIES COMPILE_FLAGS "-DHAVE_CONFIG_H")
SET(_swig_SCRIPTS ${CMAKE_CURRENT_BINARY_DIR}/SMeshHelper.py )
IF(${CMAKE_VERSION} VERSION_LESS "3.8.0")
  SWIG_ADD_MODULE(SMeshHelper python ${SMeshHelper_SOURCES})
ELSE()
  SWIG_ADD_LIBRARY(SMeshHelper LANGUAGE python SOURCES ${SMeshHelper_SOURCES})
ENDIF()
SWIG_LINK_LIBRARIES(SMeshHelper ${PYTHON_LIBRARIES} ${PLATFORM_LIBS} SMESHEngine MeshDriverGMF )
SWIG_CHECK_GENERATION(SMeshHelper)
IF(WIN32)
  SET_TARGET_PROPERTIES(_SMeshHelper PROPERTIES DEBUG_OUTPUT_NAME _SMeshHelper_d)
ENDIF(WIN32)
install(TARGETS _SMeshHelper DESTINATION ${SALOME_INSTALL_LIBS})
install(FILES ${SMeshHelper_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
SALOME_INSTALL_SCRIPTS("${_swig_SCRIPTS}"  ${SALOME_INSTALL_BINS} EXTRA_DPYS "${SWIG_MODULE_SMeshHelper_REAL_NAME}")
SALOME_INSTALL_SCRIPTS("${smesh_exe_SCRIPTS}"  ${SALOME_INSTALL_BINS})

# --- rules ---
SALOME_INSTALL_SCRIPTS("${smesh_SCRIPTS}" ${SALOME_INSTALL_PYTHON}/salome/smesh DEF_PERMS)
SALOME_INSTALL_SCRIPTS("${StdMeshers_SCRIPTS}" ${SALOME_INSTALL_PYTHON}/salome/StdMeshers DEF_PERMS)
