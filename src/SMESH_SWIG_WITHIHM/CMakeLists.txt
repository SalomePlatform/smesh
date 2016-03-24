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

INCLUDE(${SWIG_USE_FILE})

# --- options ---
# additional include directories
INCLUDE_DIRECTORIES(
  ${QT_INCLUDES}
  ${PYTHON_INCLUDE_DIRS}
  ${CAS_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${OGL_INCLUDE_DIRS}
  ${KERNEL_INCLUDE_DIRS}
  ${GUI_INCLUDE_DIRS}
  ${MEDFILE_INCLUDE_DIRS}
  ${GEOM_INCLUDE_DIRS}
  ${OMNIORB_INCLUDE_DIR}
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/src/SMESHGUI
  ${PROJECT_SOURCE_DIR}/src/OBJECT
  ${PROJECT_SOURCE_DIR}/src/SMESHDS
  ${PROJECT_SOURCE_DIR}/src/SMDS
  ${PROJECT_BINARY_DIR}/idl
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${CAS_DEFINITIONS}
  ${OMNIORB_DEFINITIONS}
  ${PYTHON_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${KERNEL_SalomeGenericObj}
  ${KERNEL_SALOMELocalTrace}
  ${GUI_CAM}
  ${GUI_suit}
  ${GUI_qtx}
  ${GUI_SalomeApp}
  ${GUI_std}
  ${GUI_Event}
  SMESH
)

# --- headers ---

# header files / to be processed by moc
SET(SMESH_Swig_HEADERS
  libSMESH_Swig.h
  libSMESH_Swig.i
)

# --- sources ---

# sources / static
SET(SMESH_Swig_SOURCES
  libSMESH_Swig.cxx
  ${SMESH_Swig_HEADERS}
)

# workaround about SWIG_ADD_MODULE bug: remove duplicates in include directories
GET_DIRECTORY_PROPERTY(_cmake_include_directories INCLUDE_DIRECTORIES)
LIST(REMOVE_DUPLICATES _cmake_include_directories)
SET_DIRECTORY_PROPERTIES(PROPERTIES INCLUDE_DIRECTORIES "${_cmake_include_directories}")

# swig flags
SET_SOURCE_FILES_PROPERTIES(libSMESH_Swig.i PROPERTIES CPLUSPLUS ON)
SET_SOURCE_FILES_PROPERTIES(libSMESH_Swig.i PROPERTIES SWIG_DEFINITIONS "-shadow")
SET_SOURCE_FILES_PROPERTIES(libSMESH_swig_wrap.cpp PROPERTIES COMPILE_FLAGS "-DHAVE_CONFIG_H")

# --- scripts ---

# scripts / swig wrappings
SET(_swig_SCRIPTS
  ${CMAKE_CURRENT_BINARY_DIR}/libSMESH_Swig.py 
)

# --- rules ---

SWIG_ADD_MODULE(libSMESH_Swig python ${SMESH_Swig_SOURCES})
SWIG_LINK_LIBRARIES(libSMESH_Swig ${_link_LIBRARIES})
IF(WIN32)
  SET_TARGET_PROPERTIES(_libSMESH_Swig PROPERTIES DEBUG_OUTPUT_NAME _libSMESH_Swig_d)
ENDIF(WIN32)

INSTALL(TARGETS _libSMESH_Swig DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${SMESH_Swig_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})

SALOME_INSTALL_SCRIPTS("${_swig_SCRIPTS}"  ${SALOME_INSTALL_BINS})
