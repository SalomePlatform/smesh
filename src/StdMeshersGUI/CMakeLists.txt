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

INCLUDE(UseQtExt)

# --- options ---
# additional include directories
INCLUDE_DIRECTORIES(
  ${QT_INCLUDES}
  ${VTK_INCLUDE_DIRS}
  ${CAS_INCLUDE_DIRS}
  ${PYTHON_INCLUDE_DIRS}
  ${KERNEL_INCLUDE_DIRS}
  ${GUI_INCLUDE_DIRS}
  ${GEOM_INCLUDE_DIRS}
  ${MEDFILE_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${QWT_INCLUDE_DIR}
  ${OMNIORB_INCLUDE_DIR}
  ${PROJECT_SOURCE_DIR}/src/SMESH
  ${PROJECT_SOURCE_DIR}/src/SMESHUtils
  ${PROJECT_SOURCE_DIR}/src/SMESH_I
  ${PROJECT_SOURCE_DIR}/src/SMESHDS
  ${PROJECT_SOURCE_DIR}/src/SMDS
  ${PROJECT_SOURCE_DIR}/src/SMESHGUI
  ${PROJECT_SOURCE_DIR}/src/Controls
  ${PROJECT_SOURCE_DIR}/src/StdMeshers
  ${PROJECT_SOURCE_DIR}/src/OBJECT
  ${PROJECT_SOURCE_DIR}/src/SMESHFiltersSelection
  ${PROJECT_BINARY_DIR}/idl
)

# additional preprocessor / compiler flags
ADD_DEFINITIONS(
  ${CAS_DEFINITIONS}
  ${BOOST_DEFINITIONS}
  ${OMNIORB_DEFINITIONS}
  ${QWT_DEFINITIONS}
)

# libraries to link to
SET(_link_LIBRARIES
  ${GUI_SalomeApp}
  ${CAS_TKernel}
  ${CAS_TKBO}
  ${CAS_TKAdvTools}
  ${QWT_LIBRARY}
  SalomeIDLSMESH
  SMESHFiltersSelection
  SMESH
  SMESHObject
)

# --- headers ---

# header files / to be processed by moc
SET(_moc_HEADERS
  StdMeshersGUI_StdHypothesisCreator.h
  StdMeshersGUI_DistrTable.h
  StdMeshersGUI_NbSegmentsCreator.h
  StdMeshersGUI_ObjectReferenceParamWdg.h
  StdMeshersGUI_QuadrangleParamWdg.h
  StdMeshersGUI_LayerDistributionParamWdg.h
  StdMeshersGUI_FixedPointsParamWdg.h
  StdMeshersGUI_SubShapeSelectorWdg.h
  StdMeshersGUI_CartesianParamCreator.h
  StdMeshersGUI_RadioButtonsGrpWdg.h
  StdMeshersGUI_PropagationHelperWdg.h
)

IF(SALOME_USE_PLOT2DVIEWER)
  LIST(APPEND _moc_HEADERS
    StdMeshersGUI_DistrPreview.h
    )
ENDIF()

# header files / no moc processing
SET(_other_HEADERS
  SMESH_StdMeshersGUI.hxx
)

# header files / to install
SET(StdMeshersGUI_HEADERS ${_moc_HEADERS} ${_other_HEADERS})

# --- sources ---

# sources / moc wrappings
QT_WRAP_MOC(_moc_SOURCES ${_moc_HEADERS})

# sources / static
SET(_other_SOURCES
  StdMeshersGUI.cxx
  StdMeshersGUI_StdHypothesisCreator.cxx
  StdMeshersGUI_DistrTable.cxx
  StdMeshersGUI_NbSegmentsCreator.cxx
  StdMeshersGUI_ObjectReferenceParamWdg.cxx
  StdMeshersGUI_QuadrangleParamWdg.cxx
  StdMeshersGUI_LayerDistributionParamWdg.cxx
  StdMeshersGUI_FixedPointsParamWdg.cxx
  StdMeshersGUI_SubShapeSelectorWdg.cxx
  StdMeshersGUI_CartesianParamCreator.cxx
  StdMeshersGUI_RadioButtonsGrpWdg.cxx
  StdMeshersGUI_PropagationHelperWdg.cxx
)

IF(SALOME_USE_PLOT2DVIEWER)
  LIST(APPEND _other_SOURCES
    StdMeshersGUI_DistrPreview.cxx
    )
ENDIF()

# sources / to compile
SET(StdMeshersGUI_SOURCES ${_other_SOURCES} ${_moc_SOURCES})

# --- resources ---

# resource files / to be processed by lrelease
SET(_ts_RESOURCES
  StdMeshers_images.ts
  StdMeshers_msg_en.ts
  StdMeshers_msg_fr.ts
  StdMeshers_msg_ja.ts
)

# --- rules ---

ADD_LIBRARY(StdMeshersGUI ${StdMeshersGUI_SOURCES})
TARGET_LINK_LIBRARIES(StdMeshersGUI ${_link_LIBRARIES} )
INSTALL(TARGETS StdMeshersGUI EXPORT ${PROJECT_NAME}TargetGroup DESTINATION ${SALOME_INSTALL_LIBS})

INSTALL(FILES ${StdMeshersGUI_HEADERS} DESTINATION ${SALOME_INSTALL_HEADERS})
QT_INSTALL_TS_RESOURCES("${_ts_RESOURCES}" "${SALOME_SMESH_INSTALL_RES_DATA}")
