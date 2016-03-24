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
  ${KERNEL_INCLUDE_DIRS}
  ${MEDFILE_INCLUDE_DIRS}
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${HDF5_DEFINITIONS}
  ${BOOST_DEFINITIONS}
)

SET(_link_LIBRARIES
  ${Boost_LIBRARIES}
)

# --- headers ---

# header files / no moc processing
SET(MEDWrapperBase_HEADERS
  MED_Common.hxx
  MED_Vector.hxx
  MED_SharedPtr.hxx
  MED_SliceArray.hxx
  MED_Wrapper.hxx
  MED_TWrapper.hxx
  MED_Structures.hxx
  MED_TStructures.hxx
  MED_Algorithm.hxx
  MED_GaussUtils.hxx
  MED_CoordUtils.hxx
  MED_Utilities.hxx
  MED_GaussDef.hxx
  MED_WrapperBase.hxx
)

# --- sources ---

# sources / static
SET(MEDWrapperBase_SOURCES
  MED_Structures.cxx
  MED_Wrapper.cxx
  MED_Algorithm.cxx
  MED_GaussUtils.cxx
  MED_CoordUtils.cxx
  MED_Utilities.cxx
  MED_GaussDef.cxx
)

# --- rules ---
 
ADD_LIBRARY(MEDWrapperBase ${MEDWrapperBase_SOURCES})
TARGET_LINK_LIBRARIES(MEDWrapperBase ${_link_LIBRARIES})
INSTALL(TARGETS MEDWrapperBase EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${MEDWrapperBase_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
