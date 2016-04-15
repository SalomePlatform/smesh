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
  ${QT_INCLUDES}
  ${CAS_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIRS}
  ${KERNEL_INCLUDE_DIRS}
  ${GUI_INCLUDE_DIRS}
  ${GEOM_INCLUDE_DIRS}
  ${MEDFILE_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${OMNIORB_INCLUDE_DIR}
  ${PROJECT_SOURCE_DIR}/src/SMESHGUI
  ${PROJECT_BINARY_DIR}/idl
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${CAS_DEFINITIONS}
  ${BOOST_DEFINITIONS}
  ${OMNIORB_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${KERNEL_SalomeIDLKernel}
  ${KERNEL_SALOMELocalTrace}
  ${KERNEL_SalomeLifeCycleCORBA}
  ${CAS_TKBRep}
  ${CAS_TKV3d}
  ${GUI_SalomeObject}
  ${GUI_suit}
  ${GUI_LightApp}
  ${GUI_SalomeApp}
  SalomeIDLSMESH
  SMESH
)

# --- headers ---

# header files / to be processed by moc
SET(GeomSelectionTools_HEADERS
  GeomSelectionTools.h
)

# --- sources ---

# sources / static
SET(GeomSelectionTools_SOURCES
  GeomSelectionTools.cxx
)

# --- rules ---

ADD_LIBRARY(GeomSelectionTools ${GeomSelectionTools_SOURCES})
TARGET_LINK_LIBRARIES(GeomSelectionTools ${_link_LIBRARIES} )
INSTALL(TARGETS GeomSelectionTools EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${GeomSelectionTools_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
