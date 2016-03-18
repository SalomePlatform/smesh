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
  ${MEDFILE_INCLUDE_DIRS}
  ${HDF5_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${KERNEL_INCLUDE_DIRS}
  ${CMAKE_CURRENT_SOURCE_DIR}/../Base
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${HDF5_DEFINITIONS}
  ${BOOST_DEFINITIONS}
)

SET(_link_LIBRARIES
  ${HDF5_LIBS}
  ${MEDFILE_C_LIBRARIES}
  ${Boost_LIBRARIES}
  MEDWrapperBase
)

# --- headers ---

# header files / no moc processing
SET(MEDWrapper_V2_2_HEADERS
  MED_V2_2_Wrapper.hxx
)

# --- sources ---

# sources / static
SET(MEDWrapper_V2_2_SOURCES
  MED_V2_2_Wrapper.cxx
)

# --- rules ---

ADD_LIBRARY(MEDWrapper_V2_2 ${MEDWrapper_V2_2_SOURCES})
TARGET_LINK_LIBRARIES(MEDWrapper_V2_2 ${_link_LIBRARIES})
INSTALL(TARGETS MEDWrapper_V2_2 EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${MEDWrapper_V2_2_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
