# Copyright (C) 2012-2020  CEA/DEN, EDF R&D
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
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${KERNEL_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${KERNEL_SALOMELocalTrace}
)

# --- headers ---

# header files
SET(ADAPTImpl_HEADERS
  HOMARD.hxx
  HOMARD_Cas.hxx
  HOMARD_Boundary.hxx
  HOMARD_Hypothesis.hxx
  HOMARD_Iteration.hxx
  HOMARD_Zone.hxx
  ADAPT_Gen.hxx
  HOMARD_DriverTools.hxx
  HOMARD_YACS.hxx
  HomardDriver.hxx
  YACSDriver.hxx
)

# --- sources ---

# sources / static
SET(ADAPTImpl_SOURCES
  HOMARD_Cas.cxx
  HOMARD_Boundary.cxx
  HOMARD_Hypothesis.cxx
  HOMARD_Iteration.cxx
  HOMARD_Zone.cxx
  ADAPT_Gen.cxx
  HOMARD_DriverTools.cxx
  HOMARD_YACS.cxx
  HomardDriver.cxx
  YACSDriver.cxx
)

# --- rules ---

ADD_LIBRARY(ADAPTImpl ${ADAPTImpl_SOURCES})
TARGET_LINK_LIBRARIES(ADAPTImpl ${_link_LIBRARIES} )
INSTALL(TARGETS ADAPTImpl EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${ADAPTImpl_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
