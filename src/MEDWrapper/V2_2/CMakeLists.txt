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
  ${MED3_INCLUDE_DIRS}
  ${HDF5_INCLUDE_DIRS}
  ${BOOST_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/../Base
  )

SET(MEDWrapper_V2_2_SOURCES
  MED_V2_2_Wrapper.cxx
  )

IF(MED_ENABLE_KERNEL)
  INCLUDE_DIRECTORIES(${KERNEL_ROOT_DIR}/include/salome)
ELSE(MED_ENABLE_KERNEL)
  INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/adm_local_without_kernel)
ENDIF(MED_ENABLE_KERNEL)

ADD_LIBRARY(MEDWrapper_V2_2 SHARED ${MEDWrapper_V2_2_SOURCES})
SET_TARGET_PROPERTIES(MEDWrapper_V2_2 PROPERTIES COMPILE_FLAGS "${HDF5_DEFINITIONS} ${MED3_DEFINITIONS} ${BOOST_DEFINITIONS} ${PLATFORM_DEFINITIONS}")
TARGET_LINK_LIBRARIES(MEDWrapper_V2_2 MEDWrapperBase ${MED3_LIBS_C_ONLY} ${HDF5_LIBS})
INSTALL(TARGETS MEDWrapper_V2_2 DESTINATION ${MED_salomelib_LIBS})

FILE(GLOB MEDWrapper_V2_2_HEADERS_HXX "${CMAKE_CURRENT_SOURCE_DIR}/*.hxx")
INSTALL(FILES ${MEDWrapper_V2_2_HEADERS_HXX} DESTINATION ${MED_salomeinclude_HEADERS})
