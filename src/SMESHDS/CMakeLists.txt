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

IF(SALOME_SMESH_USE_TBB)
  SET(TBB_INCLUDES ${TBB_INCLUDE_DIRS})
ENDIF(SALOME_SMESH_USE_TBB)

INCLUDE_DIRECTORIES(
  ${KERNEL_INCLUDE_DIRS}
  ${CAS_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/src/SMDS
  ${TBB_INCLUDES}
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${CAS_DEFINITIONS}
  ${BOOST_DEFINITIONS}
)

IF(SALOME_SMESH_USE_TBB)
  SET(TBB_LIBS ${TBB_LIBRARIES})
ENDIF(SALOME_SMESH_USE_TBB)

# libraries to link to
SET(_link_LIBRARIES
  ${CAS_KERNEL}
  ${CAS_TKBRep}
  ${KERNEL_SALOMELocalTrace}
  SMDS
  ${TBB_LIBS}
)

# --- headers ---

# header files / no moc processing
SET(SMESHDS_HEADERS
  SMESHDS_Document.hxx
  SMESHDS_Hypothesis.hxx
  SMESHDS_Mesh.hxx
  SMESHDS_Script.hxx
  SMESHDS_Command.hxx
  SMESHDS_CommandType.hxx
  SMESHDS_SubMesh.hxx
  SMESHDS_GroupBase.hxx
  SMESHDS_Group.hxx
  SMESHDS_GroupOnGeom.hxx
  SMESHDS_GroupOnFilter.hxx
  SMESH_SMESHDS.hxx
  SMESHDS_DataMapOfShape.hxx
  SMESH_Controls.hxx
)

# --- sources ---

# sources / static
SET(SMESHDS_SOURCES
  SMESHDS_Document.cxx
  SMESHDS_Hypothesis.cxx
  SMESHDS_Script.cxx
  SMESHDS_Command.cxx
  SMESHDS_SubMesh.cxx
  SMESHDS_Mesh.cxx
  SMESHDS_GroupBase.cxx
  SMESHDS_Group.cxx
  SMESHDS_GroupOnGeom.cxx
  SMESHDS_GroupOnFilter.cxx
)

# --- rules ---

ADD_LIBRARY(SMESHDS ${SMESHDS_SOURCES})
TARGET_LINK_LIBRARIES(SMESHDS ${_link_LIBRARIES} )
INSTALL(TARGETS SMESHDS EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${SMESHDS_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
