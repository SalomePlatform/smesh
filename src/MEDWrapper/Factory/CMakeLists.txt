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
  ${HDF5_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${MEDFILE_INCLUDE_DIRS}
  ${KERNEL_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/../Base
  ${CMAKE_CURRENT_SOURCE_DIR}/../V2_2
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${HDF5_DEFINITIONS}
  ${BOOST_DEFINITIONS}
)

SET(_link_LIBRARIES
  MEDWrapperBase
  MEDWrapper_V2_2
  ${Boost_LIBRARIES}
  ${MEDFILE_C_LIBRARIES}
)

# --- headers ---

# header files / no moc processing
SET(MEDWrapper_HEADERS
  MED_Factory.hxx
  MED_WrapperFactory.hxx
)

# --- sources ---

# sources / static
SET(MEDWrapper_SOURCES
  MED_Factory.cxx
)

# --- rules ---

ADD_LIBRARY(MEDWrapper ${MEDWrapper_SOURCES})
TARGET_LINK_LIBRARIES(MEDWrapper ${_link_LIBRARIES} ${MED_medloader} )
INSTALL(TARGETS MEDWrapper EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

ADD_EXECUTABLE(mprint_version mprint_version.cxx)
TARGET_LINK_LIBRARIES(mprint_version ${_link_LIBRARIES} MEDWrapper)

ADD_EXECUTABLE(MED_Test MED_Test.cxx)
TARGET_LINK_LIBRARIES(MED_Test ${_link_LIBRARIES} MEDWrapper)

INSTALL(TARGETS mprint_version MED_Test EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_BINS})

INSTALL(FILES ${MEDWrapper_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
