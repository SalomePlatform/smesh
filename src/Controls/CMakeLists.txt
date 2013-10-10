# Copyright (C) 2012-2013  CEA/DEN, EDF R&D, OPEN CASCADE
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

# --- options ---
# additional include directories
INCLUDE_DIRECTORIES(
  ${CAS_INCLUDE_DIRS}
  ${VTK_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${KERNEL_INCLUDE_DIRS}
  ${PROJECT_SOURCE_DIR}/src/SMDS
  ${PROJECT_SOURCE_DIR}/src/SMESHDS
  ${PROJECT_SOURCE_DIR}/src/SMESHUtils
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${CAS_DEFINITIONS}
  ${BOOST_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${VTK_LIBRARIES}
  ${CAS_TKernel}
  ${CAS_TKBRep}
  ${CAS_TKG3d}
  ${CAS_TKTopAlgo}
  ${CAS_TKGeomBase}
  ${CAS_TKGeomAlgo}
  SMDS
  SMESHDS
  SMESHUtils
)

SET(_link_LIBRARIES_bin
  ${KERNEL_OpUtil}
  ${KERNEL_SALOMELocalTrace}
  ${KERNEL_SALOMEBasics}
  ${KERNEL_SalomeIDLKERNEL}
  ${GEOM_GEOMAlgo}
  ${CAS_TKernel}
  ${CAS_TKBRep}
  ${CAS_TKG3d}
  SMDS
  SMESHDS
)

# --- headers ---

# header files / no moc processing
SET(SMESHControls_HEADERS
  SMESH_ControlsDef.hxx
)

# --- sources ---

# sources / static
SET(SMESHControls_SOURCES
  SMESH_Controls.cxx 
)

# bin programs
SET(SMESHControls_bin_SOURCES
  SMESHControls.cxx
)

# --- rules ---

ADD_LIBRARY(SMESHControls ${SMESHControls_SOURCES})
TARGET_LINK_LIBRARIES(SMESHControls ${_link_LIBRARIES} )
INSTALL(TARGETS SMESHControls EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

ADD_EXECUTABLE(SMESHControls_bin ${SMESHControls_bin_SOURCES})
TARGET_LINK_LIBRARIES(SMESHControls_bin ${_link_LIBRARIES_bin} SMESHControls)
SET_TARGET_PROPERTIES(SMESHControls_bin PROPERTIES OUTPUT_NAME "SMESHControls")
INSTALL(TARGETS SMESHControls_bin EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_BINS})

INSTALL(FILES ${SMESHControls_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
