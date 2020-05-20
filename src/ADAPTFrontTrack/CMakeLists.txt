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

# --- options ---
# additional include directories
INCLUDE_DIRECTORIES(
  ${OpenCASCADE_INCLUDE_DIR}
  ${GEOM_INCLUDE_DIRS}
  ${MEDFILE_INCLUDE_DIRS}
  ${MEDCOUPLING_INCLUDE_DIRS}
  ${TBB_INCLUDE_DIRS}
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${OpenCASCADE_DEFINITIONS}
)

IF(ADAPTFRONTTRACK_USE_TBB)
  SET(TBB_LIBS ${TBB_LIBRARIES})
ENDIF(ADAPTFRONTTRACK_USE_TBB)

# libraries to link to
SET(_link_LIBRARIES
  ${OpenCASCADE_FoundationClasses_LIBRARIES}
  ${OpenCASCADE_ModelingAlgorithms_LIBRARIES}
  ${OpenCASCADE_ModelingData_LIBRARIES}
  ${GEOM_XAO}
  ${MEDCoupling_medloader}
  ${TBB_LIBS}
)

# --- headers ---

# header files
SET(ADAPTFRONTTRACK_HEADERS
  FrontTrack.hxx
)

# --- sources ---

# sources / static
SET(ADAPTFRONTTRACK_SOURCES
  FrontTrack.cxx
  FrontTrack_NodeGroups.cxx
  FrontTrack_NodesOnGeom.cxx
  FrontTrack_Projector.cxx
  FrontTrack_Utils.cxx
)

# --- rules ---

ADD_LIBRARY(ADAPTFrontTrack ${ADAPTFRONTTRACK_SOURCES})
TARGET_LINK_LIBRARIES(ADAPTFrontTrack ${_link_LIBRARIES} )
INSTALL(TARGETS ADAPTFrontTrack EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${ADAPTFRONTTRACK_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
