# Copyright (C) 2012-2013  CEA/DEN, EDF R&D
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License.
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

INCLUDE_DIRECTORIES(
  ${HDF5_INCLUDE_DIRS}
  ${BOOST_INCLUDE_DIRS}
  ${MED3_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/../Base
  ${CMAKE_CURRENT_SOURCE_DIR}/../V2_2
  )

IF(MED_ENABLE_KERNEL)
  INCLUDE_DIRECTORIES(${KERNEL_ROOT_DIR}/include/salome)
ELSE(MED_ENABLE_KERNEL)
  INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/adm_local_without_kernel)
ENDIF(MED_ENABLE_KERNEL)  

SET(MEDWrapper_SOURCES
  MED_Factory.cxx
)

SET(mprint_version_SOURCES
  mprint_version.cxx
  )

ADD_LIBRARY(MEDWrapper SHARED ${MEDWrapper_SOURCES})
SET_TARGET_PROPERTIES(MEDWrapper PROPERTIES COMPILE_FLAGS "${BOOST_DEFINITIONS} ${MED3_DEFINITIONS} ${PLATFORM_DEFINITIONS}")
TARGET_LINK_LIBRARIES(MEDWrapper MEDWrapper_V2_2)
INSTALL(TARGETS MEDWrapper DESTINATION ${MED_salomelib_LIBS})

ADD_EXECUTABLE(mprint_version ${mprint_version_SOURCES})
SET_TARGET_PROPERTIES(mprint_version PROPERTIES COMPILE_FLAGS "${BOOST_DEFINITIONS} ${MED3_DEFINITIONS} ${PLATFORM_DEFINITIONS}")
TARGET_LINK_LIBRARIES(mprint_version MEDWrapper MEDWrapper_V2_2 MEDWrapperBase ${BOOST_LIBS} ${MED3_LIBS_C_ONLY})

ADD_EXECUTABLE(MED_Test ${mprint_version_SOURCES})
SET_TARGET_PROPERTIES(MED_Test PROPERTIES COMPILE_FLAGS "${BOOST_DEFINITIONS} ${MED3_DEFINITIONS} ${PLATFORM_DEFINITIONS}")
TARGET_LINK_LIBRARIES(MED_Test MEDWrapper MEDWrapper_V2_2 MEDWrapperBase ${BOOST_LIBS})

FILE(GLOB MEDWrapper_HEADERS_HXX "${CMAKE_CURRENT_SOURCE_DIR}/*.hxx")
INSTALL(FILES ${MEDWrapper_HEADERS_HXX} DESTINATION ${MED_salomeinclude_HEADERS})
INSTALL(TARGETS mprint_version MED_Test DESTINATION ${MED_salomebin_BINS})
