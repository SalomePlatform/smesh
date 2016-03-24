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
  ${HDF5_INCLUDE_DIRS}
  ${PLATFORM_INCLUDES}
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${CAS_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${CAS_TKernel}
  ${CAS_TKMath}
  ${KERNEL_SALOMELocalTrace}
)

# --- headers ---

# header files / no moc processing
SET(MEFISTO2D_HEADERS
  aptrte.h 
  Rn.h
)

# --- sources ---

IF(${F2C_FOUND})
ADD_DEFINITIONS()
  SET(F2C_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/trte.f)
  SET(F2C_OUTPUT trte.c)

  # additional include directories
  INCLUDE_DIRECTORIES(${f2c_INCLUDE_DIRS})

  # additional preprocessor / compiler flags
  ADD_DEFINITIONS(-DF2C_BUILD)


  # libraries to link to
  SET(_link_LIBRARIES ${_link_LIBRARIES} ${f2c_LIBRARIES})

  # generate C sources from Fortran
  ADD_CUSTOM_COMMAND(
    OUTPUT ${F2C_OUTPUT}
    COMMAND ${f2c_GENERATOR} ${F2C_INPUT}
    MAIN_DEPENDENCY ${F2C_INPUT}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )  
  # sources / static
  SET(MEFISTO2D_SOURCES
      aptrte.cxx
      ${F2C_OUTPUT}
  )
ELSE(${F2C_FOUND})
  # sources / static
  SET(MEFISTO2D_SOURCES
    aptrte.cxx
    trte.f
  )
ENDIF(${F2C_FOUND})

# --- rules ---

ADD_LIBRARY(MEFISTO2D ${MEFISTO2D_SOURCES})
TARGET_LINK_LIBRARIES(MEFISTO2D ${_link_LIBRARIES} )

#Ignore MSVCRT.lib on WINDOWS in case using f2c code generator
IF(WIN32)
  IF(CMAKE_BUILD_TYPE STREQUAL Debug)
    IF(${F2C_FOUND})
      SET_TARGET_PROPERTIES(MEFISTO2D PROPERTIES LINK_FLAGS "/NODEFAULTLIB:MSVCRT")
    ENDIF(${F2C_FOUND})
  ENDIF()
ENDIF(WIN32)

INSTALL(TARGETS MEFISTO2D EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})
INSTALL(FILES ${MEFISTO2D_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
