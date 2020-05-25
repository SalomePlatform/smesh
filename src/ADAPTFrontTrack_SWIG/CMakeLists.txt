# Copyright (C) 2017-2020  CEA/DEN, EDF R&D
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
  ${PYTHON_INCLUDE_DIRS}
  ${PTHREAD_INCLUDE_DIR} # pthread dependancy due to python2.7 library
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/src/ADAPTFrontTrack
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${PYTHON_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${PYTHON_LIBRARIES}
  ${PLATFORM_LIBS}
  ADAPTFrontTrack
)

# --- headers ---

# header files / to be processed by moc
SET(ADAPTFrontTrack_Swig_HEADERS
  FrontTrack.i
)

# --- sources ---

# sources / static
SET(ADAPTFrontTrack_Swig_SOURCES
  ${ADAPTFrontTrack_Swig_HEADERS}
)

# swig flags
SET_SOURCE_FILES_PROPERTIES(FrontTrack.i PROPERTIES CPLUSPLUS ON)
SET_SOURCE_FILES_PROPERTIES(FrontTrack.i PROPERTIES SWIG_DEFINITIONS "-shadow")

# --- scripts ---

# scripts / swig wrappings
SET(_swig_SCRIPTS
  ${CMAKE_CURRENT_BINARY_DIR}/FrontTrack.py
)

# --- rules ---
IF(${CMAKE_VERSION} VERSION_LESS "3.8.0")
  SWIG_ADD_MODULE(FrontTrack_Swig python ${ADAPTFrontTrack_Swig_SOURCES})
ELSE()
  SWIG_ADD_LIBRARY(FrontTrack_Swig LANGUAGE python SOURCES ${ADAPTFrontTrack_Swig_SOURCES})
ENDIF()

SWIG_LINK_LIBRARIES(FrontTrack_Swig ${_link_LIBRARIES})
SWIG_CHECK_GENERATION(FrontTrack_Swig)
IF(WIN32)
  SET_TARGET_PROPERTIES(_FrontTrack_Swig PROPERTIES DEBUG_OUTPUT_NAME _FrontTrack_Swig_d)
ENDIF(WIN32)

INSTALL(TARGETS ${SWIG_MODULE_FrontTrack_Swig_REAL_NAME} DESTINATION ${SALOME_INSTALL_PYTHON})

SALOME_INSTALL_SCRIPTS("${_swig_SCRIPTS}" ${SALOME_INSTALL_PYTHON} EXTRA_DPYS "${SWIG_MODULE_FrontTrack_Swig_REAL_NAME}")
