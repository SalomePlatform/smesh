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
  ${CAS_INCLUDE_DIRS}
  ${QT_INCLUDES}
  ${PYTHON_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${KERNEL_INCLUDE_DIRS}
  ${GUI_INCLUDE_DIRS}
  ${GEOM_INCLUDE_DIRS}
  ${MEDFILE_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${OMNIORB_INCLUDE_DIR}
  ${PROJECT_BINARY_DIR}/idl
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${OMNIORB_DEFINITIONS}
  ${PYTHON_DEFINITIONS}
  ${CAS_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${KERNEL_SalomeDSClient}
  ${KERNEL_SalomeDS}
  ${GUI_SalomeApp}
  ${GUI_suit}
  ${GUI_LightApp}
  ${GEOM_GEOM}
  ${GEOM_GEOMClient}
  ${CAS_TKernel}
  ${CAS_TKMath}
  ${CAS_TKBRep}
  SalomeIDLSMESH
)

# --- headers ---

# header files / no moc processing
SET(SMESHFiltersSelection_HEADERS
  SMESH_Type.h
  SMESH_TypeFilter.hxx
  SMESH_NumberFilter.hxx
  SMESH_LogicalFilter.hxx
)

# --- sources ---

# sources / static
SET(SMESHFiltersSelection_SOURCES
  SMESH_TypeFilter.cxx
  SMESH_NumberFilter.cxx
  SMESH_LogicalFilter.cxx
)

# --- rules ---

ADD_LIBRARY(SMESHFiltersSelection ${SMESHFiltersSelection_SOURCES})
TARGET_LINK_LIBRARIES(SMESHFiltersSelection ${_link_LIBRARIES} )
INSTALL(TARGETS SMESHFiltersSelection EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${SMESHFiltersSelection_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
